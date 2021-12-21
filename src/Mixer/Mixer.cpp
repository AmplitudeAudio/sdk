// Copyright (c) 2021-present Sparky Studios. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Core/EngineInternalState.h>
#include <Mixer/Mixer.h>
#include <Utils/Utils.h>

#include "engine_config_definition_generated.h"

#define AMPLIMIX_STORE(A, C) std::atomic_store_explicit(A, C, std::memory_order_release)
#define AMPLIMIX_LOAD(A) std::atomic_load_explicit(A, std::memory_order_acquire)
#define AMPLIMIX_CSWAP(A, E, C) std::atomic_compare_exchange_strong_explicit(A, E, C, std::memory_order_acq_rel, std::memory_order_acquire)

namespace SparkyStudios::Audio::Amplitude
{
#if defined(AM_SSE_INTRINSICS)
    constexpr AmUInt32 kSimdProcessedFramesCount = (AudioDataUnit::length);
    constexpr AmUInt32 kSimdProcessedFramesCountHalf = (kSimdProcessedFramesCount / 2);
#endif // AM_SSE_INTRINSINCS

    static hmm_vec2 LRGain(float gain, float pan)
    {
        // Clamp pan to its valid range of -1.0f to 1.0f inclusive
        pan = AM_CLAMP(pan, -1.0f, 1.0f);

        // Convert gain and pan to left and right gain and store it atomically
        // This formula is explained in the following paper:
        // http://www.rs-met.com/documents/tutorials/PanRules.pdf
        const float p = (static_cast<float>(M_PI) * (pan + 1.0f)) / 4.0f;
        const float left = std::cos(p) * gain;
        const float right = std::sin(p) * gain;

        return { left, right };
    }

    Mixer::Mixer(float masterGain)
        : _initialized(false)
        , _commandsStack()
        , _audioThreadMutex(nullptr)
        , _insideAudioThreadMutex(false)
        , _requestedBufferSize(0)
        , _requestedSampleRate(0)
        , _requestedChannels(0)
        , _deviceBufferSize(0)
        , _deviceSampleRate(0)
        , _deviceChannels(0)
        , _nextId(0)
        , _layers{}
        , _remainingFrames(0)
    {
        AMPLIMIX_STORE(&_masterGain, masterGain);
    }

    Mixer::~Mixer()
    {
        AMPLITUDE_ASSERT(!_insideAudioThreadMutex);

        if (_initialized)
        {
            _initialized = false;

            if (_audioThreadMutex)
                Thread::DestroyMutex(_audioThreadMutex);

            _audioThreadMutex = nullptr;
        }
    }

    bool Mixer::Init(const EngineConfigDefinition* config)
    {
        if (_initialized)
        {
            CallLogFunc("Amplimix has already been initialized.\n");
            return false;
        }

        _requestedBufferSize = config->output()->buffer_size();
        _requestedSampleRate = config->output()->frequency();
        _requestedChannels = config->output()->channels();

        _audioThreadMutex = Thread::CreateMutexAm();

        _initialized = true;

        return true;
    }

    void Mixer::PostInit(AmUInt32 bufferSize, AmUInt32 sampleRate, AmUInt16 channels)
    {
        _deviceBufferSize = bufferSize;
        _deviceSampleRate = sampleRate;
        _deviceChannels = channels;
    }

    AmUInt64 Mixer::Mix(AmVoidPtr mixBuffer, AmUInt64 frameCount)
    {
        LockAudioMutex();

        const AmUInt16 numChannels = _deviceChannels;
#if defined(AM_SSE_INTRINSICS)
        const AudioDataUnit lower = simdpp::make_int<AudioDataUnit>(INT16_MIN), upper = simdpp::make_int<AudioDataUnit>(INT16_MAX);
#else
        const AudioDataUnit lower = INT16_MIN, upper = INT16_MAX;
#endif // AM_SSE_INTRINSICS

        auto* buffer = static_cast<AmInt16Buffer>(mixBuffer);
        memset(buffer, 0, frameCount * numChannels * sizeof(AmInt16));

        // output remaining frames in buffer before mixing new ones
        AmUInt64 frames = frameCount;

#if defined(AM_SSE_INTRINSICS)
        // aSize in Vc::int16_v and multiple of kSimdProcessedFramesCountHalf
        const AmUInt64 aSize =
            AM_VALUE_ALIGN(frames, kSimdProcessedFramesCount) >> static_cast<AmUInt32>(std::log2(kSimdProcessedFramesCountHalf));

        // determine remaining number of frames
        _remainingFrames = aSize * kSimdProcessedFramesCountHalf - frames;
#else
        // aSize in AmInt16
        const AmUInt64 aSize = frames * numChannels;

        _remainingFrames = 0; // Should not have remaining frames without SSE optimization
#endif // AM_SSE_INTRINSICS

#if defined(AM_SSE_INTRINSICS)
        // dynamically sized buffer
        auto* align = static_cast<AudioBuffer>(
            amMemory->Malign(MemoryPoolKind::Amplimix, aSize * AudioDataUnit::length * sizeof(AmInt16), AM_SIMD_ALIGNMENT));
#else
        // dynamically sized buffer
        auto* align = static_cast<AudioBuffer>(amMemory->Malign(MemoryPoolKind::Amplimix, aSize * sizeof(AmInt16), AM_SIMD_ALIGNMENT));
#endif // AM_SSE_INTRINSICS

        // clear the aligned buffer
        for (AmUInt32 i = 0; i < aSize; i++)
        {
#if defined(AM_SSE_INTRINSICS)
            align[i] = simdpp::make_zero<AudioDataUnit>();
#else
            align[i] = 0;
#endif // AM_SSE_INTRINSICS
        }

        // begin actual mixing
        bool hasMixedAtLeastOneLayer = false;
        for (auto&& layer : _layers)
        {
            if (ShouldMix(&layer))
            {
                hasMixedAtLeastOneLayer = true;
                MixLayer(&layer, align, aSize, frames);

                // If we have mixed more frames than required, move back the cursor
                if (_remainingFrames)
                {
                    AmUInt64 cursor = AMPLIMIX_LOAD(&layer.cursor);
                    cursor -= _remainingFrames;
                    AMPLIMIX_STORE(&layer.cursor, cursor);
                }
            }
        }

        if (!hasMixedAtLeastOneLayer)
            goto Cleanup;

        // perform clipping using min and max
        for (AmUInt32 i = 0; i < aSize; i++)
        {
#if defined(AM_SSE_INTRINSICS)
            align[i] = simdpp::min(simdpp::max(align[i].eval(), lower).eval(), upper).eval();
#else
            align[i] = std::min(std::max(align[i], lower), upper);
#endif // AM_SSE_INTRINSICS
        }

        // copy frames, leaving possible remainder
        memcpy(buffer, reinterpret_cast<AmInt16*>(align), frames * numChannels * sizeof(AmInt16));

    Cleanup:
        amMemory->Free(MemoryPoolKind::Amplimix, align);

        UnlockAudioMutex();

        ExecuteCommands();

        return frameCount;
    }

    AmUInt32 Mixer::Play(SoundData* sound, PlayStateFlag flag, float gain, float pan, AmUInt32 id, AmUInt32 layer)
    {
        return PlayAdvanced(sound, flag, gain, pan, 0, sound->length, id, layer);
    }

    AmUInt32 Mixer::PlayAdvanced(
        SoundData* sound, PlayStateFlag flag, float gain, float pan, AmUInt64 startFrame, AmUInt64 endFrame, AmUInt32 id, AmUInt32 layer)
    {
        if (flag <= PLAY_STATE_FLAG_MIN || flag >= PLAY_STATE_FLAG_MAX)
            return 0; // invalid flag

#if defined(AM_SSE_INTRINSICS)
        if (endFrame - startFrame < kSimdProcessedFramesCount || endFrame < kSimdProcessedFramesCount)
            return 0; // invalid frame range
#endif // AM_SSE_INTRINSICS

        // define a layer id
        layer = layer == 0 ? ++_nextId : layer;

        // skip 0 as it is special
        if (id == 0)
            id = kAmplimixLayersCount;

        MixerCommandCallback callback = [=]() -> bool
        {
            // get layer for next sound handle id
            auto* lay = GetLayer(layer);

            // check if corresponding layer is free
            if (AMPLIMIX_LOAD(&lay->flag) == PLAY_STATE_FLAG_MIN)
            {
                // fill in non-atomic layer data along with truncating start and end
                lay->id = id;
                lay->snd = sound;

#if defined(AM_SSE_INTRINSICS)
                lay->start = startFrame & ~(kSimdProcessedFramesCount - 1);
                lay->end = endFrame & ~(kSimdProcessedFramesCount - 1);
#else
                lay->start = startFrame;
                lay->end = endFrame;
#endif // AM_SSE_INTRINSICS

                // convert gain and pan to left and right gain and store it atomically
                AMPLIMIX_STORE(&lay->gain, LRGain(gain, pan));
                // atomically set cursor to start position based on given argument
                AMPLIMIX_STORE(&lay->cursor, lay->start);
                // store flag last, releasing the layer to the mixer thread
                AMPLIMIX_STORE(&lay->flag, flag);

                // return success
                return true;
            }

            return false;
        };

        if (_insideAudioThreadMutex)
        {
            PushCommand({ callback });
            return layer;
        }

        if (callback())
            return layer;

        return 0;
    }

    bool Mixer::SetGainPan(AmUInt32 id, AmUInt32 layer, float gain, float pan)
    {
        auto* lay = GetLayer(layer);

        // check id and state flag to make sure the id is valid
        if ((id == lay->id) && (AMPLIMIX_LOAD(&lay->flag) > PLAY_STATE_FLAG_STOP))
        {
            // convert gain and pan to left and right gain and store it atomically
            AMPLIMIX_STORE(&lay->gain, LRGain(gain, pan));
            // return success
            return true;
        }

        // return failure
        return false;
    }

    bool Mixer::SetCursor(AmUInt32 id, AmUInt32 layer, AmUInt64 cursor)
    {
        auto* lay = GetLayer(layer);

        // check id and state flag to make sure the id is valid
        if ((id == lay->id) && (AMPLIMIX_LOAD(&lay->flag) > PLAY_STATE_FLAG_STOP))
        {
#if defined(AM_SSE_INTRINSICS)
            // clamp cursor and truncate to multiple of 16 before storing
            AMPLIMIX_STORE(&lay->cursor, AM_CLAMP(cursor, lay->start, lay->end) & ~(kSimdProcessedFramesCount - 1));
#else
            // clamp cursor and store it
            AMPLIMIX_STORE(&lay->cursor, AM_CLAMP(cursor, lay->start, lay->end));
#endif // AM_SSE_INTRINSICS

            // return success
            return true;
        }

        // return failure
        return false;
    }

    bool Mixer::SetPlayState(AmUInt32 id, AmUInt32 layer, PlayStateFlag flag)
    {
        // return failure if given flag invalid
        if (flag >= PLAY_STATE_FLAG_MAX)
            return false;

        MixerCommandCallback callback = [=]() -> bool
        {
            // get layer based on the lowest bits of id
            auto* lay = GetLayer(layer);

            // check id and state flag to make sure the id is valid
            if (PlayStateFlag prev; (id == lay->id) && ((prev = AMPLIMIX_LOAD(&lay->flag)) >= PLAY_STATE_FLAG_STOP))
            {
                // return success if already in desired state
                if (prev == flag)
                    return true;

                // run appropriate callback
                if (prev == PLAY_STATE_FLAG_STOP && (flag == PLAY_STATE_FLAG_PLAY || flag == PLAY_STATE_FLAG_LOOP))
                    OnSoundStarted(lay->snd);
                else if ((prev == PLAY_STATE_FLAG_PLAY || prev == PLAY_STATE_FLAG_LOOP) && flag == PLAY_STATE_FLAG_HALT)
                    OnSoundPaused(lay->snd);
                else if (prev == PLAY_STATE_FLAG_HALT && (flag == PLAY_STATE_FLAG_PLAY || flag == PLAY_STATE_FLAG_LOOP))
                    OnSoundResumed(lay->snd);
                else if ((prev == PLAY_STATE_FLAG_PLAY || prev == PLAY_STATE_FLAG_LOOP) && flag == PLAY_STATE_FLAG_STOP)
                    OnSoundStopped(lay->snd);

                // swap if flag has not changed and return if successful
                if (AMPLIMIX_CSWAP(&lay->flag, &prev, flag))
                    return true;
            }

            // return failure
            return false;
        };

        if (_insideAudioThreadMutex)
        {
            PushCommand({ callback });
            return true;
        }

        return callback();
    }

    PlayStateFlag Mixer::GetPlayState(AmUInt32 id, AmUInt32 layer)
    {
        // get layer based on the lowest bits of id
        auto* lay = GetLayer(layer);

        // check id and state flag to make sure the id is valid
        if (PlayStateFlag flag; (id == lay->id) && ((flag = AMPLIMIX_LOAD(&lay->flag)) > PLAY_STATE_FLAG_STOP))
        {
            // return the found flag
            return flag;
        }

        // return failure
        return PLAY_STATE_FLAG_MIN;
    }

    void Mixer::SetMasterGain(float gain)
    {
        AMPLIMIX_STORE(&_masterGain, gain);
    }

    void Mixer::StopAll()
    {
        // go through all active layers and set their states to the stop state
        for (auto&& lay : _layers)
        {
            // check if active and set to stop if true
            if (AMPLIMIX_LOAD(&lay.flag) > PLAY_STATE_FLAG_STOP)
                AMPLIMIX_STORE(&lay.flag, PLAY_STATE_FLAG_STOP);
        }
    }

    void Mixer::HaltAll()
    {
        // go through all playing layers and set their states to halt
        for (auto&& lay : _layers)
        {
            // check if playing or looping and try to swap
            if (PlayStateFlag flag; (flag = AMPLIMIX_LOAD(&lay.flag)) > PLAY_STATE_FLAG_HALT)
                AMPLIMIX_CSWAP(&lay.flag, &flag, PLAY_STATE_FLAG_HALT);
        }
    }

    void Mixer::PlayAll()
    {
        // go through all halted layers and set their states to play
        for (auto&& lay : _layers)
        {
            // need to reset each time
            PlayStateFlag flag = PLAY_STATE_FLAG_HALT;
            // swap the flag to play if it is on halt
            AMPLIMIX_CSWAP(&lay.flag, &flag, PLAY_STATE_FLAG_PLAY);
        }
    }

    bool Mixer::IsInsideThreadMutex() const
    {
        return _insideAudioThreadMutex;
    }

    void Mixer::PushCommand(const MixerCommand& command)
    {
        _commandsStack.push(command);
    }

    void Mixer::ExecuteCommands()
    {
        while (!_commandsStack.empty())
        {
            const auto& command = _commandsStack.front();
            command.callback();
            _commandsStack.pop();
        }
    }

    void Mixer::OnSoundStarted(const SoundData* data)
    {
        const auto* sound = static_cast<SoundInstance*>(data->userData);
        CallLogFunc("Started sound: " AM_OS_CHAR_FMT "\n", sound->GetSound()->GetFilename());
    }

    void Mixer::OnSoundPaused(const SoundData* data)
    {
        const auto* sound = static_cast<SoundInstance*>(data->userData);
        CallLogFunc("Paused sound: " AM_OS_CHAR_FMT "\n", sound->GetSound()->GetFilename());
    }

    void Mixer::OnSoundResumed(const SoundData* data)
    {
        const auto* sound = static_cast<SoundInstance*>(data->userData);
        CallLogFunc("Resumed sound: " AM_OS_CHAR_FMT "\n", sound->GetSound()->GetFilename());
    }

    void Mixer::OnSoundStopped(const SoundData* data)
    {
        const auto* sound = static_cast<SoundInstance*>(data->userData);
        CallLogFunc("Stopped sound: " AM_OS_CHAR_FMT "\n", sound->GetSound()->GetFilename());
    }

    bool Mixer::OnSoundLooped(const SoundData* data)
    {
        auto* sound = static_cast<SoundInstance*>(data->userData);
        CallLogFunc("Looped sound: " AM_OS_CHAR_FMT "\n", sound->GetSound()->GetFilename());

        ++sound->_currentLoopCount;

        if (const AmUInt32 loopCount = sound->GetSettings().m_loopCount; sound->_currentLoopCount == loopCount)
        {
            sound->GetChannel()->Halt();
            return false;
        }

        return true;
    }

    AmUInt64 Mixer::OnSoundStream(const SoundData* data, AmUInt64 offset, AmUInt64 frames)
    {
        if (data->stream)
        {
            const auto* sound = static_cast<SoundInstance*>(data->userData);
            return sound->GetAudio(offset, frames);
        }

        return 0;
    }

    void Mixer::OnSoundEnded(const SoundData* data)
    {
        const auto* sound = static_cast<SoundInstance*>(data->userData);
        CallLogFunc("Ended sound: " AM_OS_CHAR_FMT "\n", sound->GetSound()->GetFilename());

        RealChannel* channel = sound->GetChannel();

        if (const Engine* engine = Engine::GetInstance(); engine->GetState()->stopping)
            goto Delete;

        if (sound->GetSettings().m_kind == SoundKind::Standalone)
        {
            goto Stop;
        }
        else if (sound->GetSettings().m_kind == SoundKind::Switched)
        {
            goto Delete;
        }
        else if (sound->GetSettings().m_kind == SoundKind::Contained)
        {
            const Collection* collection = sound->GetCollection();
            AMPLITUDE_ASSERT(collection != nullptr); // Should always have a collection for contained sound instances.

            if (const CollectionDefinition* config = collection->GetCollectionDefinition();
                config->play_mode() == CollectionPlayMode_PlayAll)
            {
                if (channel->Valid())
                {
                    channel->MarkAsPlayed(sound->GetSound());
                    if (channel->AllSoundsHasPlayed())
                    {
                        channel->ClearPlayedSounds();
                        if (config->play_mode() == CollectionPlayMode_PlayAll)
                        {
                            goto Stop;
                        }
                    }

                    // Play the collection again only if the channel is still playing.
                    if (channel->Playing())
                    {
                        channel->GetParentChannelState()->Play();
                    }
                }

                // Delete the current sound instance.
                goto Delete;
            }
        }
        else
        {
            AMPLITUDE_ASSERT(false); // Should never fall in this case.
        }

    Stop:
        // Stop playing the sound
        channel->GetParentChannelState()->Halt();

    Delete:
        // Delete the sound instance at the end of the playback
        delete sound;
    }

    void Mixer::OnSoundDestroyed(const SoundData* data)
    {
        MixerCommandCallback callback = [data]() -> bool
        {
            auto* sound = static_cast<SoundInstance*>(data->userData);
            sound->Destroy();
            return true;
        };

        if (_insideAudioThreadMutex)
        {
            PushCommand({ callback });
            return;
        }

        callback();
    }

    void Mixer::MixLayer(MixerLayer* layer, AudioBuffer buffer, AmUInt64 bufferSize, AmUInt64 samples)
    {
        const AmUInt16 numChannels = _deviceChannels;

        // load flag value atomically first
        PlayStateFlag flag = AMPLIMIX_LOAD(&layer->flag);

        // atomically load cursor
        AmUInt64 cursor = AMPLIMIX_LOAD(&layer->cursor);

        // atomically load left and right gain
        const hmm_vec2 g = AMPLIMIX_LOAD(&layer->gain);
        const float gain = AMPLIMIX_LOAD(&_masterGain);
#if defined(AM_SSE_INTRINSICS)
        const auto mxGain = simdpp::make_int<AudioDataUnit>(AmFloatToFixedPoint(g.X * gain), AmFloatToFixedPoint(g.Y * gain));
        const auto& lGain = mxGain;
        const auto& rGain = mxGain;
#else
        const auto lGain = AmFloatToFixedPoint(g.X * gain);
        const auto rGain = AmFloatToFixedPoint(g.Y * gain);
#endif // AM_SSE_INTRINSICS

        // loop state
        const bool loop = (flag == PLAY_STATE_FLAG_LOOP);

        // if this sound is streaming and we have a stream event callback
        if (layer->snd != nullptr && layer->snd->stream == true)
        {
            // mix sound per chunk of streamed data
            AmUInt64 c = samples;
            while (c > 0 && flag != PLAY_STATE_FLAG_MIN)
            {
                // update flag value
                flag = AMPLIMIX_LOAD(&layer->flag);

                if (flag == PLAY_STATE_FLAG_MIN)
                    break;

                const AmUInt64 chunkSize = AM_MIN(layer->snd->chunk->frames, c);
#if defined(AM_SSE_INTRINSICS)
                const AmUInt64 aChunkSize =
                    AM_VALUE_ALIGN(chunkSize, kSimdProcessedFramesCount) >> static_cast<AmUInt32>(std::log2(kSimdProcessedFramesCountHalf));
#else
                const AmUInt64 aChunkSize = chunkSize * numChannels;
#endif // AM_SSE_INTRINSICS
                const AmUInt64 len = OnSoundStream(layer->snd, cursor, chunkSize);

                // having 0 here mainly means that we have reached
                // the end of the stream and the audio is not looping.
                if (len == 0)
                    break;

                auto* buf = reinterpret_cast<AudioBuffer>(reinterpret_cast<AmInt16*>(buffer) + (samples - c) * numChannels);

                // action based on flag
                if (flag >= PLAY_STATE_FLAG_PLAY)
                {
                    // PLAY_STATE_FLAG_PLAY or PLAY_STATE_FLAG_LOOP, play including fade in
                    cursor = layer->snd->format.GetNumChannels() == 1 ? MixMono(layer, loop, cursor, lGain, rGain, buf, aChunkSize)
                                                                      : MixStereo(layer, loop, cursor, lGain, rGain, buf, aChunkSize);

                    // clear flag if PLAY_STATE_FLAG_PLAY and the cursor has reached the end
                    if ((flag == PLAY_STATE_FLAG_PLAY) && (cursor == layer->end))
                        AMPLIMIX_CSWAP(&layer->flag, &flag, PLAY_STATE_FLAG_MIN);
                }

                c -= len;
            }
        }
        else if (layer->snd != nullptr && !layer->snd->stream)
        {
            // action based on flag
            if (flag >= PLAY_STATE_FLAG_PLAY)
            {
                // PLAY_STATE_FLAG_PLAY or PLAY_STATE_FLAG_LOOP, play including fade in
                cursor = layer->snd->format.GetNumChannels() == 1 ? MixMono(layer, loop, cursor, lGain, rGain, buffer, bufferSize)
                                                                  : MixStereo(layer, loop, cursor, lGain, rGain, buffer, bufferSize);

                // clear flag if PLAY_STATE_FLAG_PLAY and the cursor has reached the end
                if ((flag == PLAY_STATE_FLAG_PLAY) && (cursor == layer->end))
                    AMPLIMIX_CSWAP(&layer->flag, &flag, PLAY_STATE_FLAG_MIN);
            }
        }

        // run callback if reached the end
        if (cursor == layer->end)
        {
            // We are in the audio thread mutex here
            MixerCommandCallback callback = [=]() -> bool
            {
                // stop playback unless looping
                if (!loop)
                {
                    OnSoundEnded(layer->snd);
                }
                else
                {
                    // call the onLoop callback
                    if (OnSoundLooped(layer->snd))
                    {
                        // wrap around if allowed to loop again
                        AMPLIMIX_CSWAP(&layer->cursor, &layer->end, layer->start);
                    }
                    else
                    {
                        // stop playback
                        OnSoundEnded(layer->snd);
                    }
                }

                return true;
            };

            // Postpone call outside the audio thread mutex
            PushCommand({ callback });
        }
    }

    AmUInt64 Mixer::MixMono(
        MixerLayer* layer,
        bool loop,
        AmUInt64 cursor,
        const AudioDataUnit& lGain,
        const AudioDataUnit& rGain,
        AudioBuffer buffer,
        const AmUInt64& bufferSize)
    {
        // cache cursor
        AmUInt64 old = cursor;

        // regular playback
        for (AmUInt64 i = 0; i < bufferSize; i += 2)
        {
            // check if cursor at end
            if (cursor == layer->end)
            {
                // quit unless looping
                if (!loop)
                    break;

                // call the onLoop callback
                if (OnSoundLooped(layer->snd))
                {
                    // wrap around if allowed to loop again
                    cursor = layer->start;
                }
                else
                {
                    // stop playback
                    break;
                }
            }

            AmUInt64 off;

            if (layer->snd->stream)
            {
                // when streaming we always fit the data buffer,
                // otherwise something is wrong
                off = i / 2;
            }
            else
            {
#if defined(AM_SSE_INTRINSICS)
                off = (cursor % layer->snd->length) >> static_cast<AmUInt32>(std::log2(kSimdProcessedFramesCountHalf) + 1);
#else
                off = (cursor % layer->snd->length);
#endif // AM_SSE_INTRINSICS
            }

            AudioDataUnit sample = layer->snd->chunk->buffer[off];

#if defined(AM_SSE_INTRINSICS)
            buffer[i + 0] =
                simdpp::add(
                    buffer[i + 0],
                    simdpp::to_int16(
                        simdpp::shift_r(simdpp::mull(simdpp::zip_lo(sample, sample).eval(), lGain).eval(), kAmFixedPointShift).eval())
                        .eval())
                    .eval();
            buffer[i + 1] =
                simdpp::add(
                    buffer[i + 1],
                    simdpp::to_int16(
                        simdpp::shift_r(simdpp::mull(simdpp::zip_hi(sample, sample).eval(), rGain).eval(), kAmFixedPointShift).eval())
                        .eval())
                    .eval();
#else
            buffer[i] += static_cast<AmInt16>((sample * lGain) >> kAmFixedPointShift);
            buffer[i + 1] += static_cast<AmInt16>((sample * rGain) >> kAmFixedPointShift);
#endif // AM_SSE_INTRINSICS

            // advance cursor
#if defined(AM_SSE_INTRINSICS)
            cursor += kSimdProcessedFramesCount;
#else
            cursor++;
#endif // AM_SSE_INTRINSICS
        }

        // swap back cursor if unchanged
        if (!AMPLIMIX_CSWAP(&layer->cursor, &old, cursor))
            cursor = old;

        // return new cursor
        return cursor;
    }

    AmUInt64 Mixer::MixStereo(
        MixerLayer* layer,
        bool loop,
        AmUInt64 cursor,
        const AudioDataUnit& lGain,
        const AudioDataUnit& rGain,
        AudioBuffer buffer,
        const AmUInt64& bufferSize)
    {
        // cache cursor
        AmUInt64 old = cursor;

        // regular playback
        for (AmUInt64 i = 0; i < bufferSize; i += 2)
        {
            // check if cursor at end
            if (cursor == layer->end)
            {
                // quit unless looping
                if (!loop)
                    break;

                // call the onLoop callback
                if (OnSoundLooped(layer->snd))
                {
                    // wrap around if allowed to loop again
                    cursor = layer->start;
                }
                else
                {
                    // stop playback
                    break;
                }
            }

            AmUInt64 off;

            if (layer->snd->stream)
            {
                // when streaming we always fit the data buffer,
                // otherwise something is wrong
                off = i;
            }
            else
            {
#if defined(AM_SSE_INTRINSICS)
                off = (cursor % layer->snd->length) >> static_cast<AmUInt32>(std::log2(kSimdProcessedFramesCountHalf));
#else
                off = (cursor % layer->snd->length) << 1;
#endif // AM_SSE_INTRINSICS
            }

#if defined(AM_SSE_INTRINSICS)
            buffer[i + 0] =
                simdpp::add(
                    buffer[i + 0],
                    simdpp::to_int16(
                        simdpp::shift_r(simdpp::mull(layer->snd->chunk->buffer[off + 0], lGain).eval(), kAmFixedPointShift).eval())
                        .eval())
                    .eval();
            buffer[i + 1] =
                simdpp::add(
                    buffer[i + 1],
                    simdpp::to_int16(
                        simdpp::shift_r(simdpp::mull(layer->snd->chunk->buffer[off + 1], rGain).eval(), kAmFixedPointShift).eval())
                        .eval())
                    .eval();
#else
            buffer[i] += static_cast<AmInt16>((layer->snd->chunk->buffer[off] * lGain) >> kAmFixedPointShift);
            buffer[i + 1] += static_cast<AmInt16>((layer->snd->chunk->buffer[off + 1] * rGain) >> kAmFixedPointShift);
#endif // AM_SSE_INTRINSICS

            // advance cursor
#if defined(AM_SSE_INTRINSICS)
            cursor += kSimdProcessedFramesCount;
#else
            cursor++;
#endif // AM_SSE_INTRINSICS
        }

        // swap back cursor if unchanged
        if (!AMPLIMIX_CSWAP(&layer->cursor, &old, cursor))
            cursor = old;

        // return new cursor
        return cursor;
    }

    MixerLayer* Mixer::GetLayer(AmUInt32 layer)
    {
        // get layer based on the lowest bits of layer id
        return &_layers[layer & kAmplimixLayersMask];
    }

    bool Mixer::ShouldMix(MixerLayer* layer)
    {
        // load flag value
        PlayStateFlag flag = AMPLIMIX_LOAD(&layer->flag);

        // return if flag is not cleared
        return (flag > PLAY_STATE_FLAG_HALT);
    }

    void Mixer::LockAudioMutex()
    {
        AMPLITUDE_ASSERT(!_insideAudioThreadMutex);

        if (_audioThreadMutex)
        {
            Thread::LockMutex(_audioThreadMutex);
        }

        _insideAudioThreadMutex = true;
    }

    void Mixer::UnlockAudioMutex()
    {
        AMPLITUDE_ASSERT(_insideAudioThreadMutex);

        if (_audioThreadMutex)
        {
            Thread::UnlockMutex(_audioThreadMutex);
        }

        _insideAudioThreadMutex = false;
    }
} // namespace SparkyStudios::Audio::Amplitude
