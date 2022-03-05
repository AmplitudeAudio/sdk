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
        , _pipeline(nullptr)
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

        if (const auto* pipeline = config->mixer()->pipeline(); pipeline != nullptr && pipeline->size() > 0)
        {
            _pipeline = new ProcessorPipeline();

            for (flatbuffers::uoffset_t i = 0; i < pipeline->size(); ++i)
            {
                switch (config->mixer()->pipeline_type()->Get(i))
                {
                case AudioMixerPipelineItem_AudioProcessorMixer:
                    {
                        const AudioProcessorMixer* p = pipeline->GetAs<AudioProcessorMixer>(i);
                        SoundProcessor* dryProcessor = SoundProcessor::Find(p->dry_processor()->str());
                        SoundProcessor* wetProcessor = SoundProcessor::Find(p->wet_processor()->str());

                        if (dryProcessor == nullptr)
                        {
                            CallLogFunc(
                                "[WARNING] Unable to find a registered sound processor with name: %s\n", p->dry_processor()->c_str());
                            continue;
                        }

                        if (wetProcessor == nullptr)
                        {
                            CallLogFunc(
                                "[WARNING] Unable to find a registered sound processor with name: %s\n", p->wet_processor()->c_str());
                            continue;
                        }

                        ProcessorMixer* mixer = new ProcessorMixer();
                        mixer->SetDryProcessor(dryProcessor, p->dry());
                        mixer->SetWetProcessor(wetProcessor, p->wet());

                        _pipeline->Append(mixer);
                    }
                    break;

                case AudioMixerPipelineItem_AudioSoundProcessor:
                    {
                        const AudioSoundProcessor* p = pipeline->GetAs<AudioSoundProcessor>(i);
                        SoundProcessor* soundProcessor = SoundProcessor::Find(p->processor()->str());
                        if (soundProcessor == nullptr)
                        {
                            CallLogFunc("[WARNING] Unable to find a registered sound processor with name: %s\n", p->processor()->c_str());
                            continue;
                        }

                        _pipeline->Append(soundProcessor);
                    }
                    break;
                }
            }
        }

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
        const auto lower = simdpp::make_int<AudioDataUnit>(INT16_MIN), upper = simdpp::make_int<AudioDataUnit>(INT16_MAX);
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
                UpdatePitch(&layer);

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

    AmUInt32 Mixer::Play(
        SoundData* sound, PlayStateFlag flag, float gain, float pan, AmReal32 pitch, AmReal32 speed, AmUInt32 id, AmUInt32 layer)
    {
        return PlayAdvanced(sound, flag, gain, pan, pitch, speed, 0, sound->length, id, layer);
    }

    AmUInt32 Mixer::PlayAdvanced(
        SoundData* sound,
        PlayStateFlag flag,
        float gain,
        float pan,
        AmReal32 pitch,
        AmReal32 speed,
        AmUInt64 startFrame,
        AmUInt64 endFrame,
        AmUInt32 id,
        AmUInt32 layer)
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

                // Initialize the sample rate converter
                if (lay->sampleRateConverter != nullptr)
                    src_delete(lay->sampleRateConverter);

                lay->sampleRateConverter = src_new(amEngine->GetSampleRateConversionQuality(), sound->format.GetNumChannels(), nullptr);

                if (lay->sampleRateConverter == nullptr)
                    return false;

                // convert gain and pan to left and right gain and store it atomically
                AMPLIMIX_STORE(&lay->gain, LRGain(gain, pan));
                // store the pitch
                AMPLIMIX_STORE(&lay->pitch, pitch);
                // store the playback speed
                AMPLIMIX_STORE(&lay->userPlaySpeed, speed);
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

    bool Mixer::SetPitch(AmUInt32 id, AmUInt32 layer, AmReal32 pitch)
    {
        auto* lay = GetLayer(layer);

        // check id and state flag to make sure the id is valid
        if ((id == lay->id) && (AMPLIMIX_LOAD(&lay->flag) > PLAY_STATE_FLAG_STOP))
        {
            // convert gain and pan to left and right gain and store it atomically
            AMPLIMIX_STORE(&lay->pitch, pitch);
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

    bool Mixer::SetPlaySpeed(AmUInt32 id, AmUInt32 layer, AmReal32 speed)
    {
        auto* lay = GetLayer(layer);

        // check id and state flag to make sure the id is valid
        if ((id == lay->id) && (AMPLIMIX_LOAD(&lay->flag) > PLAY_STATE_FLAG_STOP))
        {
            // convert gain and pan to left and right gain and store it atomically
            AMPLIMIX_STORE(&lay->userPlaySpeed, speed);
            // return success
            return true;
        }

        // return failure
        return false;
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
            if (command.callback)
            {
                command.callback();
            }
            _commandsStack.pop();
        }
    }

    void Mixer::OnSoundStarted(const SoundData* data)
    {
        const auto* sound = static_cast<SoundInstance*>(data->userData);
        CallLogFunc("Started sound: " AM_OS_CHAR_FMT "\n", sound->GetSound()->GetFilename().c_str());
    }

    void Mixer::OnSoundPaused(const SoundData* data)
    {
        const auto* sound = static_cast<SoundInstance*>(data->userData);
        CallLogFunc("Paused sound: " AM_OS_CHAR_FMT "\n", sound->GetSound()->GetFilename().c_str());
    }

    void Mixer::OnSoundResumed(const SoundData* data)
    {
        const auto* sound = static_cast<SoundInstance*>(data->userData);
        CallLogFunc("Resumed sound: " AM_OS_CHAR_FMT "\n", sound->GetSound()->GetFilename().c_str());
    }

    void Mixer::OnSoundStopped(const SoundData* data)
    {
        const auto* sound = static_cast<SoundInstance*>(data->userData);
        CallLogFunc("Stopped sound: " AM_OS_CHAR_FMT "\n", sound->GetSound()->GetFilename().c_str());
    }

    bool Mixer::OnSoundLooped(const SoundData* data)
    {
        auto* sound = static_cast<SoundInstance*>(data->userData);
        CallLogFunc("Looped sound: " AM_OS_CHAR_FMT "\n", sound->GetSound()->GetFilename().c_str());

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
        auto* sound = static_cast<SoundInstance*>(data->userData);
        CallLogFunc("Ended sound: " AM_OS_CHAR_FMT "\n", sound->GetSound()->GetFilename().c_str());

        RealChannel* channel = sound->GetChannel();

        // Clean up the pipeline
        _pipeline->Cleanup(sound);

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

        const AmUInt16 channels = layer->snd->format.GetNumChannels();
        const AmUInt32 baseSampleRate = layer->snd->format.GetSampleRate();
        const AmUInt32 sampleRate = AMPLIMIX_LOAD(&layer->sampleRate);
        /* */ AmReal32 step = static_cast<AmReal32>(baseSampleRate) / static_cast<AmReal32>(sampleRate);

        // avoid step overflow
        if (step > (1 << (32 - kAmFixedPointBits)))
            step = 0;

        /* */ AmUInt64 inSamples = std::ceil(step * static_cast<AmReal32>(samples));
#if defined(AM_SSE_INTRINSICS)
        inSamples = AM_VALUE_ALIGN(inSamples, kSimdProcessedFramesCount);
#endif // AM_SSE_INTRINSICS

        SoundChunk* in = SoundChunk::CreateChunk(inSamples, channels, MemoryPoolKind::Amplimix);

        // if this sound is streaming, and we have a stream event callback
        if (layer->snd != nullptr && layer->snd->stream)
        {
            // mix sound per chunk of streamed data
            AmUInt64 c = inSamples;
            while (c > 0 && flag != PLAY_STATE_FLAG_MIN)
            {
                // update flag value
                flag = AMPLIMIX_LOAD(&layer->flag);

                if (flag == PLAY_STATE_FLAG_MIN)
                    break;

                const AmUInt64 chunkSize = AM_MIN(layer->snd->chunk->frames, c);
                /* */ AmUInt64 readLen = chunkSize;

#if defined(AM_SSE_INTRINSICS)
                readLen = AM_VALUE_ALIGN(readLen, kSimdProcessedFramesCount);
#endif // AM_SSE_INTRINSICS

                readLen = OnSoundStream(layer->snd, (cursor + (inSamples - c)) % layer->snd->length, readLen);
                readLen = AM_MIN(readLen, chunkSize);

                // having 0 here mainly means that we have reached
                // the end of the stream and the audio is not looping.
                if (readLen == 0)
                    break;

                memcpy(
                    reinterpret_cast<AmInt16Buffer>(in->buffer) + ((inSamples - c) * channels),
                    reinterpret_cast<AmInt16Buffer>(layer->snd->chunk->buffer), readLen * channels * sizeof(AmInt16));

                c -= readLen;
            }
        }
        else if (layer->snd != nullptr && !layer->snd->stream)
        {
            // Compute offset
            const AmUInt64 offset = (cursor % layer->snd->length) * channels;
            const AmUInt64 remaining = layer->snd->chunk->frames - cursor;

            if (cursor < layer->snd->chunk->frames && remaining < samples)
            {
                const AmUInt64 size = remaining * layer->snd->format.GetFrameSize();

                memcpy(
                    reinterpret_cast<AmInt16Buffer>(in->buffer), reinterpret_cast<AmInt16Buffer>(layer->snd->chunk->buffer) + offset, size);

                memcpy(
                    reinterpret_cast<AmInt16Buffer>(in->buffer) + (remaining * channels),
                    reinterpret_cast<AmInt16Buffer>(layer->snd->chunk->buffer), in->size - size);
            }
            else
            {
                memcpy(
                    reinterpret_cast<AmInt16Buffer>(in->buffer), reinterpret_cast<AmInt16Buffer>(layer->snd->chunk->buffer) + offset,
                    in->size);
            }
        }

        // action based on flag
        if (flag >= PLAY_STATE_FLAG_PLAY)
        {
            // PLAY_STATE_FLAG_PLAY or PLAY_STATE_FLAG_LOOP, play including fade in
            cursor = channels == 1
                ? MixMono(layer, loop, cursor, lGain, rGain, in->buffer, in->length, inSamples, buffer, bufferSize, samples)
                : MixStereo(layer, loop, cursor, lGain, rGain, in->buffer, in->length, inSamples, buffer, bufferSize, samples);

            // clear flag if PLAY_STATE_FLAG_PLAY and the cursor has reached the end
            if ((flag == PLAY_STATE_FLAG_PLAY) && (cursor == layer->end))
                AMPLIMIX_CSWAP(&layer->flag, &flag, PLAY_STATE_FLAG_MIN);
        }

        SoundChunk::DestroyChunk(in);

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
                        // wrap around if allowed looping again
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
        AudioBuffer inBuffer,
        const AmUInt64& inBufferSize,
        const AmUInt64& inSamples,
        AudioBuffer outBuffer,
        const AmUInt64& outBufferSize,
        const AmUInt64& outSamples)
    {
        // Process pipeline
        if (_pipeline == nullptr)
        {
            CallLogFunc("[WARNING] No active pipeline is set, this means no sound will be rendered. You should configure the Amplimix "
                        "pipeline in your engine configuration file.\n");
            return cursor;
        }

        // Cache cursor
        AmUInt64 oldCursor = cursor;

        const AmUInt16 channels = layer->snd->format.GetNumChannels();
        const AmUInt32 baseSampleRate = layer->snd->format.GetSampleRate();
        const AmUInt32 sampleRate = AMPLIMIX_LOAD(&layer->sampleRate);
        /* */ AmReal32 step = static_cast<AmReal32>(baseSampleRate) / static_cast<AmReal32>(sampleRate);

        // avoid step overflow
        if (step > (1 << (32 - kAmFixedPointBits)))
            step = 0;

        const AmInt32 stepFixed = AmFloatToFixedPoint(step);

        /* */ AmUInt64 samples = outSamples;
        /* */ AmUInt64 bufferSize = outBufferSize;

        SoundChunk* out = SoundChunk::CreateChunk(outSamples, channels, MemoryPoolKind::Amplimix);

        if (sampleRate != baseSampleRate)
        {
            const AmUInt64 inputL = inSamples * channels;
            const AmUInt64 outputL = outSamples * channels;

            auto* inputI16 = reinterpret_cast<AmInt16Buffer>(inBuffer);
            auto* outputI16 = reinterpret_cast<AmInt16Buffer>(out->buffer);

            auto* inputR32 =
                static_cast<AmReal32Buffer>(amMemory->Malign(MemoryPoolKind::Amplimix, inputL * sizeof(AmReal32), AM_SIMD_ALIGNMENT));
            auto* outputR32 =
                static_cast<AmReal32Buffer>(amMemory->Malign(MemoryPoolKind::Amplimix, outputL * sizeof(AmReal32), AM_SIMD_ALIGNMENT));

            memset(outputR32, 0, outputL * sizeof(AmReal32));

            for (AmUInt64 i = 0; i < inputL; i++)
            {
                inputR32[i] = AmInt16ToReal32(inputI16[i]);
            }

            SRC_DATA data;
            data.data_in = inputR32;
            data.data_out = outputR32;
            data.end_of_input = 0;
            data.input_frames = inSamples;
            data.output_frames = outSamples;
            data.src_ratio = static_cast<AmReal64>(sampleRate) / baseSampleRate;

            src_process(layer->sampleRateConverter, &data);

            for (AmUInt64 i = 0; i < outputL; i++)
            {
                outputI16[i] = AmReal32ToInt16(outputR32[i], true);
            }

            amMemory->Free(MemoryPoolKind::Amplimix, inputR32);
            amMemory->Free(MemoryPoolKind::Amplimix, outputR32);
        }
        else
        {
            memcpy(out->buffer, inBuffer, out->size);
        }

        switch (layer->snd->format.GetInterleaveType())
        {
        case AM_SAMPLE_INTERLEAVED:
            _pipeline->ProcessInterleaved(
                reinterpret_cast<AmInt16Buffer>(out->buffer), reinterpret_cast<AmInt16Buffer>(out->buffer), samples, out->size, channels,
                sampleRate, static_cast<SoundInstance*>(layer->snd->userData));
            break;
        case AM_SAMPLE_NON_INTERLEAVED:
            _pipeline->Process(
                reinterpret_cast<AmInt16Buffer>(out->buffer), reinterpret_cast<AmInt16Buffer>(out->buffer), samples, out->size, channels,
                sampleRate, static_cast<SoundInstance*>(layer->snd->userData));
            break;
        default:
            CallLogFunc("[WARNING] A bad sound data interleave type was encountered.\n");
            break;
        }

        AmUInt64 position = cursor << kAmFixedPointBits;

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
                    // wrap around if allowed looping again
                    cursor = layer->start;
                }
                else
                {
                    // stop playback
                    break;
                }
            }

            AudioDataUnit sample = out->buffer[i / 2];

#if defined(AM_SSE_INTRINSICS)
            outBuffer[i + 0] = simdpp::add(
                outBuffer[i + 0],
                simdpp::to_int16(simdpp::shift_r(simdpp::mull(simdpp::zip_lo(sample, sample), lGain), kAmFixedPointBits)));
            outBuffer[i + 1] = simdpp::add(
                outBuffer[i + 1],
                simdpp::to_int16(simdpp::shift_r(simdpp::mull(simdpp::zip_hi(sample, sample), rGain), kAmFixedPointBits)));

            position += stepFixed * kSimdProcessedFramesCount;
#else
            outBuffer[i + 0] += static_cast<AmInt16>((sample * lGain) >> kAmFixedPointBits);
            outBuffer[i + 1] += static_cast<AmInt16>((sample * rGain) >> kAmFixedPointBits);

            position += stepFixed;
#endif // AM_SSE_INTRINSICS
        }

        cursor = position >> kAmFixedPointBits;

#if defined(AM_SSE_INTRINSICS)
        cursor = AM_VALUE_ALIGN(cursor, kSimdProcessedFramesCount);
#endif // AM_SSE_INTRINSICS

        // swap back cursor if unchanged
        if (!AMPLIMIX_CSWAP(&layer->cursor, &oldCursor, cursor))
            cursor = oldCursor;

        SoundChunk::DestroyChunk(out);

        // return new cursor
        return cursor;
    }

    AmUInt64 Mixer::MixStereo(
        MixerLayer* layer,
        bool loop,
        AmUInt64 cursor,
        const AudioDataUnit& lGain,
        const AudioDataUnit& rGain,
        AudioBuffer inBuffer,
        const AmUInt64& inBufferSize,
        const AmUInt64& inSamples,
        AudioBuffer outBuffer,
        const AmUInt64& outBufferSize,
        const AmUInt64& outSamples)
    {
        // Process pipeline
        if (_pipeline == nullptr)
        {
            CallLogFunc("[WARNING] No active pipeline is set, this means no sound will be rendered. You should configure the Amplimix "
                        "pipeline in your engine configuration file.\n");
            return cursor;
        }

        // Cache cursor and position
        AmUInt64 oldCursor = cursor;

        const AmUInt16 channels = layer->snd->format.GetNumChannels();
        const AmUInt32 baseSampleRate = layer->snd->format.GetSampleRate();
        const AmUInt32 sampleRate = AMPLIMIX_LOAD(&layer->sampleRate);
        /* */ AmReal32 step = static_cast<AmReal32>(baseSampleRate) / static_cast<AmReal32>(sampleRate);

        // avoid step overflow
        if (step > (1 << (32 - kAmFixedPointBits)))
            step = 0;

        const AmInt32 stepFixed = AmFloatToFixedPoint(step);

        /* */ AmUInt64 samples = outSamples;
        /* */ AmUInt64 bufferSize = outBufferSize;

        SoundChunk* out = SoundChunk::CreateChunk(outSamples, channels, MemoryPoolKind::Amplimix);

        if (sampleRate != baseSampleRate)
        {
            const AmUInt64 inputL = inSamples * channels;
            const AmUInt64 outputL = outSamples * channels;

            auto* inputI16 = reinterpret_cast<AmInt16Buffer>(inBuffer);
            auto* outputI16 = reinterpret_cast<AmInt16Buffer>(out->buffer);

            auto* inputR32 =
                static_cast<AmReal32Buffer>(amMemory->Malign(MemoryPoolKind::Amplimix, inputL * sizeof(AmReal32), AM_SIMD_ALIGNMENT));
            auto* outputR32 =
                static_cast<AmReal32Buffer>(amMemory->Malign(MemoryPoolKind::Amplimix, outputL * sizeof(AmReal32), AM_SIMD_ALIGNMENT));

            memset(outputR32, 0, outputL * sizeof(AmReal32));

            for (AmUInt64 i = 0; i < inputL; i++)
            {
                inputR32[i] = AmInt16ToReal32(inputI16[i]);
            }

            SRC_DATA data;
            data.data_in = inputR32;
            data.data_out = outputR32;
            data.end_of_input = 0;
            data.input_frames = inSamples;
            data.output_frames = outSamples;
            data.src_ratio = static_cast<AmReal64>(sampleRate) / baseSampleRate;

            src_process(layer->sampleRateConverter, &data);

            for (AmUInt64 i = 0; i < outputL; i++)
            {
                outputI16[i] = AmReal32ToInt16(outputR32[i], true);
            }

            amMemory->Free(MemoryPoolKind::Amplimix, inputR32);
            amMemory->Free(MemoryPoolKind::Amplimix, outputR32);
        }
        else
        {
            memcpy(out->buffer, inBuffer, out->size);
        }

        switch (layer->snd->format.GetInterleaveType())
        {
        case AM_SAMPLE_INTERLEAVED:
            _pipeline->ProcessInterleaved(
                reinterpret_cast<AmInt16Buffer>(out->buffer), reinterpret_cast<AmInt16Buffer>(out->buffer), samples, out->size, channels,
                sampleRate, static_cast<SoundInstance*>(layer->snd->userData));
            break;
        case AM_SAMPLE_NON_INTERLEAVED:
            _pipeline->Process(
                reinterpret_cast<AmInt16Buffer>(out->buffer), reinterpret_cast<AmInt16Buffer>(out->buffer), samples, out->size, channels,
                sampleRate, static_cast<SoundInstance*>(layer->snd->userData));
            break;
        default:
            CallLogFunc("[WARNING] A bad sound data interleave type was encountered.\n");
            break;
        }

        AmUInt64 position = cursor << kAmFixedPointBits;

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
                    // wrap around if allowed looping again
                    cursor = layer->start;
                }
                else
                {
                    // stop playback
                    break;
                }
            }

#if defined(AM_SSE_INTRINSICS)
            outBuffer[i + 0] = simdpp::add(
                outBuffer[i + 0], simdpp::to_int16(simdpp::shift_r(simdpp::mull(out->buffer[i + 0], lGain), kAmFixedPointBits)));
            outBuffer[i + 1] = simdpp::add(
                outBuffer[i + 1], simdpp::to_int16(simdpp::shift_r(simdpp::mull(out->buffer[i + 1], rGain), kAmFixedPointBits)));

            position += stepFixed * kSimdProcessedFramesCount;
#else
            outBuffer[i] += static_cast<AmInt16>((out->buffer[i + 0] * lGain) >> kAmFixedPointBits);
            outBuffer[i + 1] += static_cast<AmInt16>((out->buffer[i + 1] * rGain) >> kAmFixedPointBits);

            position += stepFixed;
#endif // AM_SSE_INTRINSICS
        }

        cursor = position >> kAmFixedPointBits;

#if defined(AM_SSE_INTRINSICS)
        cursor = AM_VALUE_ALIGN(cursor, kSimdProcessedFramesCount);
#endif // AM_SSE_INTRINSICS

        // swap back cursor if unchanged
        if (!AMPLIMIX_CSWAP(&layer->cursor, &oldCursor, cursor))
            cursor = oldCursor;

        // Clean pipeline buffer if any
        SoundChunk::DestroyChunk(out);

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

    void Mixer::UpdatePitch(MixerLayer* layer)
    {
        auto* sound = static_cast<SoundInstance*>(layer->snd->userData);

        const AmReal32 pitch = AMPLIMIX_LOAD(&layer->pitch);
        const AmReal32 speed = AMPLIMIX_LOAD(&layer->userPlaySpeed);

        if (pitch > 0)
        {
            const AmReal32 playSpeed = pitch * speed;
            AMPLIMIX_STORE(&layer->playSpeed, playSpeed);
            AMPLIMIX_STORE(&layer->sampleRate, playSpeed * _deviceSampleRate);
        }
        else
        {
            constexpr AmReal32 playSpeed = 0.001f;
            AMPLIMIX_STORE(&layer->playSpeed, playSpeed);
            AMPLIMIX_STORE(&layer->sampleRate, playSpeed * _deviceSampleRate);
        }
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
