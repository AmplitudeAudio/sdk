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

#include "engine_config_definition_generated.h"

#define AMPLIMIX_STORE(A, C) std::atomic_store_explicit(A, C, std::memory_order_release)
#define AMPLIMIX_LOAD(A) std::atomic_load_explicit(A, std::memory_order_acquire)
#define AMPLIMIX_CSWAP(A, E, C) std::atomic_compare_exchange_strong_explicit(A, E, C, std::memory_order_acq_rel, std::memory_order_acquire)

#define AMPLIMIX_FX_BITS (16)
#define AMPLIMIX_FX_UNIT (1 << AMPLIMIX_FX_BITS)
#define AMPLIMIX_FX_MASK (AMPLIMIX_FX_UNIT - 1)

#define AMPLIMIX_FX_FROM_FLOAT(f) (AmInt16((f)*AMPLIMIX_FX_UNIT))

namespace SparkyStudios::Audio::Amplitude
{
    static hmm_vec2 LRGain(float gain, float pan)
    {
        // Clamp pan to its valid range of -1.0f to 1.0f inclusive
        pan = AM_CLAMP(pan, -1.0f, 1.0f);

        // Convert gain and pan to left and right gain and store it atomically
        // This formula is explained in the following paper:
        // http://www.rs-met.com/documents/tutorials/PanRules.pdf
        const float p = (static_cast<float>(M_PI) * (pan + 1.0f)) / 4.0f;
        const float left = std::cosf(p) * gain;
        const float right = std::sinf(p) * gain;

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
        , _oldFrames{}
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

    void Mixer::PostInit(AmUInt32 bufferSize, AmUInt32 sampleRate, AmUInt32 channels)
    {
        _deviceBufferSize = bufferSize;
        _deviceSampleRate = sampleRate;
        _deviceChannels = channels;
    }

    AmUInt32 Mixer::Mix(AmVoidPtr mixBuffer, AmUInt32 frameCount)
    {
        LockAudioMutex();

        AmUInt32 numChannels = _deviceChannels;

        auto* buffer = static_cast<AmInt16Buffer>(mixBuffer);
        memset(buffer, 0, frameCount * numChannels * sizeof(AmInt16));

        // output remaining frames in buffer before mixing new ones
        AmUInt32 rnum = frameCount;

        // only do this if there are old frames
        if (_remainingFrames)
        {
            if (rnum > _remainingFrames)
            {
                // rnum bigger than remaining frames (usual case)
                memcpy(buffer, _oldFrames, _remainingFrames * numChannels * sizeof(AmInt16));
                rnum -= _remainingFrames;
                buffer += _remainingFrames * numChannels;
                _remainingFrames = 0;
            }
            else
            {
                // rnum smaller equal remaining frames (rare case)
                memcpy(buffer, _oldFrames, rnum * numChannels * sizeof(AmInt16));
                _remainingFrames -= rnum;

                // move back remaining old frames if any
                if (_remainingFrames)
                    memmove(_oldFrames, _oldFrames + rnum * numChannels, (3 - rnum) * numChannels * sizeof(AmInt16));

                // return
                return frameCount;
            }
        }

#if defined(AM_SSE_INTRINSICS)
        // aSize in Vc::int16_v and multiple of 4
        AmUInt32 aSize = AM_VALUE_ALIGN(rnum, 4) >> 3;
#else
        // aSize in AmInt16
        AmUInt32 aSize = rnum * numChannels;
#endif // AM_SSE_INTRINSICS

        // dynamically sized aligned buffer
        AudioBuffer align = new AudioDataUnit[aSize];

        // clear the aligned buffer using SSE assignment
        for (AmUInt32 i = 0; i < aSize; i++)
            align[i] = AudioDataUnit(0);

        // begin actual mixing, caching the volume first
        for (auto& layer : _layers)
        {
            MixLayer(&layer, align, aSize, rnum);
        }

        // perform clipping using SSE min and max
        auto neg1 = AudioDataUnit(INT16_MIN), pos1 = AudioDataUnit(INT16_MAX);
        for (AmUInt32 i = 0; i < aSize; i++)
        {
            align[i] = (std::min)((std::max)(align[i], neg1), pos1);
        }

        // copy rnum frames, leaving possible remainder
        memcpy(buffer, reinterpret_cast<AmInt16*>(align), rnum * numChannels * sizeof(AmInt16));

        // determine remaining number of frames
#if defined(AM_SSE_INTRINSICS)
        _remainingFrames = aSize * 8 - rnum;
#else
        _remainingFrames = 0; // Should not have remaining frames without SSE optimization
#endif // AM_SSE_INTRINSICS

        // copy remaining frames to buffer inside the mixer struct
        if (_remainingFrames)
            memcpy(_oldFrames, reinterpret_cast<AmInt16*>(align) + rnum * numChannels, _remainingFrames * numChannels * sizeof(AmInt16));

        delete[] align;

        UnlockAudioMutex();

        ExecuteCommands();

        return frameCount;
    }

    AmUInt32 Mixer::Play(SoundData* sound, PlayStateFlag flag, float gain, float pan, AmUInt32 id, AmUInt32 layer)
    {
        return PlayAdvanced(sound, flag, gain, pan, 0, sound->length, id, layer);
    }

    AmUInt32 Mixer::PlayAdvanced(
        SoundData* sound, PlayStateFlag flag, float gain, float pan, AmUInt32 startFrame, AmUInt32 endFrame, AmUInt32 id, AmUInt32 layer)
    {
        if (flag <= PLAY_STATE_FLAG_MIN || flag >= PLAY_STATE_FLAG_MAX)
            return 0; // invalid flag

        if (endFrame - startFrame < 16 || endFrame < 16)
            return 0; // invalid frame range

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
                lay->start = startFrame & ~15;
                lay->end = endFrame & ~15;

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

    bool Mixer::SetCursor(AmUInt32 id, AmUInt32 layer, AmUInt32 cursor)
    {
        auto* lay = GetLayer(layer);

        // check id and state flag to make sure the id is valid
        if ((id == lay->id) && (AMPLIMIX_LOAD(&lay->flag) > PLAY_STATE_FLAG_STOP))
        {
            // clamp cursor and truncate to multiple of 16 before storing
            AMPLIMIX_STORE(&lay->cursor, AM_CLAMP(cursor, lay->start, lay->end) & ~15);
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

        const AmUInt32 loopCount = sound->GetSettings().m_loopCount;
        if (sound->_currentLoopCount == loopCount)
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

        if (const Engine* engine = Engine::GetInstance(); engine->GetState()->stopping)
            goto Delete;

        RealChannel* channel = sound->GetChannel();

        if (sound->GetSettings().m_kind == SoundKind::Standalone)
            goto Stop;

        else if (sound->GetSettings().m_kind == SoundKind::Switched)
            goto Delete;

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
            AMPLITUDE_ASSERT(false); // Should never fall in this case.

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

    void Mixer::MixLayer(MixerLayer* layer, AudioBuffer buffer, AmUInt32 bufferSize, AmUInt32 samples)
    {
        AmUInt32 numChannels = _deviceChannels;

        // load flag value atomically first
        PlayStateFlag flag = AMPLIMIX_LOAD(&layer->flag);

        // return if flag cleared
        if (flag == PLAY_STATE_FLAG_MIN)
            return;

        // atomically load cursor
        AmUInt32 cursor = AMPLIMIX_LOAD(&layer->cursor);

        // atomically load left and right gain
        const hmm_vec2 g = AMPLIMIX_LOAD(&layer->gain);
        const float gain = AMPLIMIX_LOAD(&_masterGain);
        const auto lGain = AudioDataUnit(AMPLIMIX_FX_FROM_FLOAT(g.X * gain));
        const auto rGain = AudioDataUnit(AMPLIMIX_FX_FROM_FLOAT(g.Y * gain));

        // if this sound is streaming and we have a stream event callback
        if (layer->snd != nullptr && layer->snd->stream)
        {
            // mix sound per chunk of streamed data
            AmUInt32 c = samples;
            while (c > 0 && flag != PLAY_STATE_FLAG_MIN)
            {
                // update flag value
                flag = AMPLIMIX_LOAD(&layer->flag);

                if (flag == PLAY_STATE_FLAG_MIN)
                    break;

                AmUInt64 chunkSize = AM_MIN(layer->snd->chunk->frames, c);
#if defined(AM_SSE_INTRINSICS)
                AmUInt64 aChunkSize = AM_VALUE_ALIGN(chunkSize, 4) >> 3;
#else
                AmUInt64 aChunkSize = chunkSize * numChannels;
#endif // AM_SSE_INTRINSICS
                AmUInt64 len = OnSoundStream(layer->snd, cursor, chunkSize);

                // having 0 here mainly means that we have reached
                // the end of the stream and the audio is not looping.
                if (len == 0)
                    break;

                auto* buf = reinterpret_cast<AudioBuffer>(reinterpret_cast<AmInt16*>(buffer) + (samples - c) * numChannels);

                // action based on flag
                if (flag >= PLAY_STATE_FLAG_PLAY)
                {
                    // PLAY_STATE_FLAG_PLAY or PLAY_STATE_FLAG_LOOP, play including fade in
                    cursor = layer->snd->format.GetNumChannels() == 1
                        ? MixMono(layer, (flag == PLAY_STATE_FLAG_LOOP), cursor, lGain, rGain, buf, aChunkSize)
                        : MixStereo(layer, (flag == PLAY_STATE_FLAG_LOOP), cursor, lGain, rGain, buf, aChunkSize);

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
                cursor = layer->snd->format.GetNumChannels() == 1
                    ? MixMono(layer, (flag == PLAY_STATE_FLAG_LOOP), cursor, lGain, rGain, buffer, bufferSize)
                    : MixStereo(layer, (flag == PLAY_STATE_FLAG_LOOP), cursor, lGain, rGain, buffer, bufferSize);

                // clear flag if PLAY_STATE_FLAG_PLAY and the cursor has reached the end
                if ((flag == PLAY_STATE_FLAG_PLAY) && (cursor == layer->end))
                    AMPLIMIX_CSWAP(&layer->flag, &flag, PLAY_STATE_FLAG_MIN);
            }
        }

        // run callback if reached the end
        if (cursor == layer->end)
            OnSoundEnded(layer->snd);
    }

    AmUInt32 Mixer::MixMono(
        MixerLayer* layer, bool loop, AmUInt32 cursor, AudioDataUnit lGain, AudioDataUnit rGain, AudioBuffer buffer, AmUInt32 bufferSize)
    {
        // cache cursor
        AmUInt32 old = cursor;

        // regular playback
        for (AmUInt32 i = 0; i < bufferSize; i += 2)
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

            AmUInt32 off;

            if (layer->snd->stream)
            {
                // when streaming we always fit the data buffer,
                // otherwise something is wrong
                off = i / 2;
            }
            else
            {
#if defined(AM_SSE_INTRINSICS)
                off = (cursor % layer->snd->length) >> 4;
#else
                off = (cursor % layer->snd->length);
#endif // AM_SSE_INTRINSICS
            }

            AudioDataUnit sample = layer->snd->chunk->buffer[off];

#if defined(AM_SSE_INTRINSICS)
            auto gain = lGain.interleaveLow(rGain);
            AmInt32 j = 0;
            // buffer[i] = std::fma(sample.interleaveLow(sample), gain, buffer[i]);
            // buffer[i + 1] = std::fma(sample.interleaveHigh(sample), gain, buffer[i + 1]);
            buffer[i] += sample.interleaveLow(sample).apply(
                [gain, &j](AmInt16 sample) -> AmInt16
                {
                    return (sample * gain[j++]) >> AMPLIMIX_FX_BITS;
                });
            j = 0;
            buffer[i + 1] += sample.interleaveHigh(sample).apply(
                [gain, &j](AmInt16 sample) -> AmInt16
                {
                    return (sample * gain[j++]) >> AMPLIMIX_FX_BITS;
                });
#else
            buffer[i] += (sample * lGain) >> AMPLIMIX_FX_BITS;
            buffer[i + 1] += (sample * rGain) >> AMPLIMIX_FX_BITS;
#endif // AM_SSE_INTRINSICS

            // advance cursor
#if defined(AM_SSE_INTRINSICS)
            cursor += 16;
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

    AmUInt32 Mixer::MixStereo(
        MixerLayer* layer, bool loop, AmUInt32 cursor, AudioDataUnit lGain, AudioDataUnit rGain, AudioBuffer buffer, AmUInt32 bufferSize)
    {
        // cache cursor
        AmUInt32 old = cursor;

        // regular playback
        for (AmUInt32 i = 0; i < bufferSize; i += 2)
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

            AmUInt32 off;

            if (layer->snd->stream)
            {
                // when streaming we always fit the data buffer,
                // otherwise something is wrong
                off = i;
            }
            else
            {
#if defined(AM_SSE_INTRINSICS)
                off = (cursor % layer->snd->length) >> 3;
#else
                off = (cursor % layer->snd->length) << 1;
#endif // AM_SSE_INTRINSICS
            }

#if defined(AM_SSE_INTRINSICS)
            auto gain = lGain.interleaveLow(rGain);
            AmInt32 j = 0;
            // buffer[i] = std::fma(layer->snd->chunk->buffer[off], gain, buffer[i]);
            // buffer[i + 1] = std::fma(layer->snd->chunk->buffer[off + 1], gain, buffer[i + 1]);
            buffer[i] += layer->snd->chunk->buffer[off].apply(
                [gain, &j](AmInt16 sample) -> AmInt16
                {
                    return (sample * gain[j++]) >> AMPLIMIX_FX_BITS;
                });
            j = 0;
            buffer[i + 1] += layer->snd->chunk->buffer[off + 1].apply(
                [gain, &j](AmInt16 sample) -> AmInt16
                {
                    return (sample * gain[j++]) >> AMPLIMIX_FX_BITS;
                });
#else
            buffer[i] += (layer->snd->chunk->buffer[off] * lGain) >> AMPLIMIX_FX_BITS;
            buffer[i + 1] += (layer->snd->chunk->buffer[off + 1] * rGain) >> AMPLIMIX_FX_BITS;
#endif // AM_SSE_INTRINSICS

            // advance cursor
#if defined(AM_SSE_INTRINSICS)
            cursor += 16;
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
