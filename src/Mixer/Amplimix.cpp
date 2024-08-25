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

#include <Core/Engine.h>
#include <Mixer/Amplimix.h>

#define AMPLIMIX_STORE(A, C) std::atomic_store_explicit(A, (C), std::memory_order_release)
#define AMPLIMIX_LOAD(A) std::atomic_load_explicit(A, std::memory_order_acquire)
#define AMPLIMIX_CSWAP(A, E, C) std::atomic_compare_exchange_strong_explicit(A, E, C, std::memory_order_acq_rel, std::memory_order_acquire)

namespace SparkyStudios::Audio::Amplitude
{
    struct AmplimixMutexLocker
    {
        explicit AmplimixMutexLocker(AmplimixImpl* mixer)
            : m_mixer(mixer)
        {
            Lock();
        }

        ~AmplimixMutexLocker()
        {
            Unlock();
        }

        [[nodiscard]] bool IsLocked() const
        {
            return m_locked;
        }

        void Lock()
        {
            if (IsLocked())
                return;

            m_mixer->LockAudioMutex();
            m_locked = true;
        }

        void Unlock()
        {
            if (!IsLocked())
                return;

            m_mixer->UnlockAudioMutex();
            m_locked = false;
        }

    private:
        AmplimixImpl* m_mixer;
        bool m_locked = false;
    };

    constexpr AmUInt32 kProcessedFramesCount = GetSimdBlockSize();

    static void OnSoundDestroyed(AmplimixImpl* mixer, AmplimixLayerImpl* layer);

    static AmVec2 LRGain(AmReal32 gain, AmReal32 pan)
    {
        // Clamp pan to its valid range of -1.0f to 1.0f inclusive
        pan = AM_CLAMP(pan, -1.0f, 1.0f);

        // Convert gain and pan to left and right gain and store it atomically
        // This formula is explained in the following paper:
        // http://www.rs-met.com/documents/tutorials/PanRules.pdf
        const AmReal32 p = static_cast<AmReal32>(M_PI) * (pan + 1.0f) / 4.0f;
        const AmReal32 left = std::cos(p) * gain;
        const AmReal32 right = std::sin(p) * gain;

        return { left, right };
    }

    static bool ShouldLoopSound(AmplimixImpl* mixer, AmplimixLayerImpl* layer)
    {
        const auto* sound = layer->snd->sound.get();
        const AmUInt32 loopCount = sound->GetSettings().m_loopCount;

        return sound->GetCurrentLoopCount() != loopCount;
    }

    static void OnSoundStarted(AmplimixImpl* mixer, AmplimixLayerImpl* layer)
    {
        const auto* sound = layer->snd->sound.get();
        amLogDebug("Started sound: '" AM_OS_CHAR_FMT "'.", sound->GetSound()->GetPath().c_str());

        const auto channel = sound->GetChannel();
        auto* channelState = channel.GetState();

        channelState->Trigger(ChannelEvent::Begin);
    }

    static void OnSoundPaused(AmplimixImpl* mixer, AmplimixLayerImpl* layer)
    {
        const auto* sound = layer->snd->sound.get();
        amLogDebug("Paused sound: '" AM_OS_CHAR_FMT "'.", sound->GetSound()->GetPath().c_str());

        const auto channel = sound->GetChannel();
        auto* channelState = channel.GetState();

        channelState->Trigger(ChannelEvent::Pause);
    }

    static void OnSoundResumed(AmplimixImpl* mixer, AmplimixLayerImpl* layer)
    {
        const auto* sound = layer->snd->sound.get();
        amLogDebug("Resumed sound: '" AM_OS_CHAR_FMT "'.", sound->GetSound()->GetPath().c_str());

        const auto channel = sound->GetChannel();
        auto* channelState = channel.GetState();

        channelState->Trigger(ChannelEvent::Resume);
    }

    static void OnSoundStopped(AmplimixImpl* mixer, AmplimixLayerImpl* layer)
    {
        const auto* sound = layer->snd->sound.get();
        amLogDebug("Stopped sound: '" AM_OS_CHAR_FMT "'.", sound->GetSound()->GetPath().c_str());

        const auto channel = sound->GetChannel();
        auto* channelState = channel.GetState();

        channelState->Trigger(ChannelEvent::Stop);

        // Destroy the sound instance on stop
        OnSoundDestroyed(mixer, layer);
    }

    static bool OnSoundLooped(AmplimixImpl* mixer, AmplimixLayerImpl* layer)
    {
        auto* sound = layer->snd->sound.get();
        amLogDebug("Looped sound: '" AM_OS_CHAR_FMT "'.", sound->GetSound()->GetPath().c_str());

        AmplimixImpl::IncrementSoundLoopCount(sound);

        const bool shouldLoop = ShouldLoopSound(mixer, layer);

        if (shouldLoop)
        {
            const auto channel = sound->GetChannel();
            auto* channelState = channel.GetState();

            channelState->Trigger(ChannelEvent::Loop);
        }

        return shouldLoop;
    }

    static AmUInt64 OnSoundStream(AmplimixImpl* mixer, AmplimixLayerImpl* layer, AmUInt64 offset, AmUInt64 frames)
    {
        if (!layer->snd->stream)
            return 0;

        const auto* sound = layer->snd->sound.get();
        return sound->GetAudio(offset, frames);
    }

    static void OnSoundEnded(AmplimixImpl* mixer, AmplimixLayerImpl* layer)
    {
        auto* sound = layer->snd->sound.get();
        amLogDebug("Ended sound: '" AM_OS_CHAR_FMT "'.", sound->GetSound()->GetPath().c_str());

        const auto channel = sound->GetChannel();
        auto* channelState = channel.GetState();

        if (const auto* engine = static_cast<const EngineImpl*>(Engine::GetInstance()); engine->GetState()->stopping)
        {
            channelState->Trigger(ChannelEvent::End);

            OnSoundDestroyed(mixer, layer);
            return;
        }

        if (sound->GetSettings().m_kind == SoundKind::Standalone)
        {
            // Stop playing the sound
            channelState->HaltInternal();

            channelState->Trigger(ChannelEvent::End);

            // Destroy the sound instance on end
            OnSoundDestroyed(mixer, layer);
        }
        else if (sound->GetSettings().m_kind == SoundKind::Switched)
        {
            // Stop playing the sound
            channelState->HaltInternal();

            channelState->Trigger(ChannelEvent::End);

            // Destroy the sound instance on stop
            OnSoundDestroyed(mixer, layer);
        }
        else if (sound->GetSettings().m_kind == SoundKind::Contained)
        {
            const CollectionImpl* collection = sound->GetCollection();
            AMPLITUDE_ASSERT(collection != nullptr); // Should always have a collection for contained sound instances.

            if (const CollectionDefinition* config = collection->GetDefinition(); config->play_mode() == CollectionPlayMode_PlayAll)
            {
                if (channelState->Valid())
                {
                    channelState->GetRealChannel().MarkAsPlayed(sound->GetSound());
                    if (channelState->GetRealChannel().AllSoundsHasPlayed())
                    {
                        channelState->GetRealChannel().ClearPlayedSounds();
                        if (config->play_mode() == CollectionPlayMode_PlayAll)
                        {
                            // Stop playing the collection
                            channelState->HaltInternal();

                            channelState->Trigger(ChannelEvent::End);
                        }
                    }

                    // Play the collection again only if the channel is still playing.
                    if (channelState->GetRealChannel().Playing())
                    {
                        channelState->Play();
                    }
                }

                // Delete the current sound instance.
                OnSoundDestroyed(mixer, layer);
            }
        }
        else
        {
            AMPLITUDE_ASSERT(false); // Should never fall in this case.
        }
    }

    static void OnSoundDestroyed(AmplimixImpl* mixer, AmplimixLayerImpl* layer)
    {
        if (layer->snd == nullptr)
            return;

        const auto* sound = layer->snd->sound.get();

        // Clean up the pipeline
        mixer->GetPipeline()->Cleanup(layer);

        layer->snd->sound.reset();

        layer->snd = nullptr;

        AMPLIMIX_STORE(&layer->flag, ePSF_MIN);
    }

    static void MixMono(AmUInt64 index, const AmAudioFrame& gain, const AudioBufferChannel& in, AudioBufferChannel& out)
    {
#if defined(AM_SIMD_INTRINSICS)
        const auto x = xsimd::load_aligned(&in[index]);
        const auto y = xsimd::load_aligned(&out[index]);

        xsimd::store_aligned(&out[index], xsimd::fma(x, gain, y));
#else
        out[index] += in[index] * gain;
#endif // AM_SIMD_INTRINSICS
    }

    AmplimixImpl::AmplimixImpl(AmReal32 masterGain)
        : _initialized(false)
        , _commandsStack()
        , _audioThreadMutex(nullptr)
        , _insideAudioThreadMutex()
        , _nextId(0)
        , _masterGain()
        , _layers()
        , _remainingFrames(0)
        , _pipeline(nullptr)
        , _device()
        , _scratchBuffer(kAmMaxSupportedFrameCount, kAmMaxSupportedChannelCount)
    {
        AMPLIMIX_STORE(&_masterGain, masterGain);
    }

    AmplimixImpl::~AmplimixImpl()
    {
        Deinit(); // Ensures Deinit is called
        _scratchBuffer.Clear();
    }

    bool AmplimixImpl::Init(const EngineConfigDefinition* config)
    {
        if (_initialized)
        {
            amLogError("Amplimix has already been initialized.");
            return false;
        }

        if (const auto* pipeline = config->mixer()->pipeline(); pipeline != nullptr && pipeline->size() > 0)
        {
            _pipeline.reset(ampoolnew(MemoryPoolKind::Amplimix, Pipeline));

            for (flatbuffers::uoffset_t i = 0, l = pipeline->size(); i < l; ++i)
            {
                switch (config->mixer()->pipeline_type()->Get(i))
                {
                case AudioMixerPipelineItem_AudioProcessorMixer:
                    {
                        const auto* p = pipeline->GetAs<AudioProcessorMixer>(i);
                        SoundProcessorInstance* dryProcessor = SoundProcessor::Construct(p->dry_processor()->str());
                        SoundProcessorInstance* wetProcessor = SoundProcessor::Construct(p->wet_processor()->str());

                        if (dryProcessor == nullptr)
                        {
                            amLogError("Unable to find a registered sound processor with name: %s.", p->dry_processor()->c_str());
                            _pipeline.reset(nullptr);
                            return false;
                        }

                        if (wetProcessor == nullptr)
                        {
                            amLogError("Unable to find a registered sound processor with name: %s.", p->wet_processor()->c_str());
                            _pipeline.reset(nullptr);
                            return false;
                        }

                        auto* mixer = ampoolnew(MemoryPoolKind::Amplimix, ProcessorMixer);
                        mixer->SetDryProcessor(dryProcessor, p->dry());
                        mixer->SetWetProcessor(wetProcessor, p->wet());

                        _pipeline->Append(mixer);
                    }
                    break;

                case AudioMixerPipelineItem_AudioSoundProcessor:
                    {
                        const auto* p = pipeline->GetAs<AudioSoundProcessor>(i);
                        SoundProcessorInstance* soundProcessor = SoundProcessor::Construct(p->processor()->str());
                        if (soundProcessor == nullptr)
                        {
                            amLogError("Unable to find a registered sound processor with name: %s.", p->processor()->c_str());
                            _pipeline.reset(nullptr);
                            return false;
                        }

                        _pipeline->Append(soundProcessor);
                    }
                    break;

                default:
                    AMPLITUDE_ASSERT(false);
                    break;
                }
            }
        }

        if (_pipeline == nullptr)
        {
            amLogCritical("Invalid pipeline configuration.");
            return false;
        }

        _device.mOutputBufferSize = config->output()->buffer_size();
        _device.mRequestedOutputSampleRate = config->output()->frequency();
        _device.mRequestedOutputChannels = static_cast<PlaybackOutputChannels>(config->output()->channels());
        _device.mRequestedOutputFormat = static_cast<PlaybackOutputFormat>(config->output()->format());

        _audioThreadMutex = Thread::CreateMutex(500);

        _initialized = true;

        return true;
    }

    void AmplimixImpl::Deinit()
    {
        if (!_initialized)
            return;

        AMPLITUDE_ASSERT(!IsInsideThreadMutex());

        _initialized = false;

        if (_audioThreadMutex)
            Thread::DestroyMutex(_audioThreadMutex);

        _audioThreadMutex = nullptr;

        _pipeline.reset(nullptr);

        for (auto& layer : _layers)
            layer.Reset();
    }

    void AmplimixImpl::UpdateDevice(
        AmObjectID deviceID,
        AmString deviceName,
        AmUInt32 deviceOutputSampleRate,
        PlaybackOutputChannels deviceOutputChannels,
        PlaybackOutputFormat deviceOutputFormat)
    {
        _device.mDeviceID = deviceID;
        _device.mDeviceName = std::move(deviceName);
        _device.mDeviceOutputSampleRate = deviceOutputSampleRate;
        _device.mDeviceOutputChannels = deviceOutputChannels;
        _device.mDeviceOutputFormat = deviceOutputFormat;
    }

    AmUInt64 AmplimixImpl::Mix(AudioBuffer** outBuffer, AmUInt64 frameCount)
    {
        if (outBuffer != nullptr)
            // Enforce the output buffer to be null before calling Mix
            *outBuffer = nullptr;

        if (!_initialized || amEngine->GetState() == nullptr || amEngine->GetState()->stopping || amEngine->GetState()->paused)
            return 0;

        AmplimixMutexLocker lock(this);

        // clear the output buffer
        _scratchBuffer.Clear();

        // determine remaining number of frames
#if defined(AM_SIMD_INTRINSICS)
        _remainingFrames = AM_VALUE_ALIGN(frameCount, GetSimdBlockSize()) - frameCount;
#else
        _remainingFrames = 0; // Should not have remaining frames without SIMD optimization
#endif // AM_SIMD_INTRINSICS

        // begin actual mixing
        bool hasMixedAtLeastOneLayer = false;
        for (auto&& layer : _layers)
        {
            if (!ShouldMix(&layer))
                continue;

            UpdatePitch(&layer);

            hasMixedAtLeastOneLayer = true;
            MixLayer(&layer, &_scratchBuffer, frameCount);

#if defined(AM_SIMD_ALIGNMENT)
            // If we have mixed more frames than required, move back the cursor
            if (_remainingFrames > 0)
            {
                AmUInt64 cursor = AMPLIMIX_LOAD(&layer.cursor);
                cursor -= _remainingFrames;
                AMPLIMIX_STORE(&layer.cursor, cursor);
            }
#endif // AM_SIMD_ALIGNMENT
        }

        lock.Unlock();

        ExecuteCommands();

        if (hasMixedAtLeastOneLayer)
        {
            if (outBuffer != nullptr)
                *outBuffer = &_scratchBuffer;

            return frameCount;
        }

        return 0;
    }

    AmUInt32 AmplimixImpl::Play(
        SoundData* sound, PlayStateFlag flag, AmReal32 gain, AmReal32 pan, AmReal32 pitch, AmReal32 speed, AmUInt32 id, AmUInt32 layer)
    {
        return PlayAdvanced(sound, flag, gain, pan, pitch, speed, 0, sound->length, id, layer);
    }

    AmUInt32 AmplimixImpl::PlayAdvanced(
        SoundData* sound,
        PlayStateFlag flag,
        AmReal32 gain,
        AmReal32 pan,
        AmReal32 pitch,
        AmReal32 speed,
        AmUInt64 startFrame,
        AmUInt64 endFrame,
        AmUInt32 id,
        AmUInt32 layer)
    {
        if (flag <= ePSF_MIN || flag >= ePSF_MAX)
            return 0; // invalid flag

        if (endFrame - startFrame < kProcessedFramesCount || endFrame < kProcessedFramesCount)
            return 0; // invalid frame range

        // define a layer id
        layer = layer == 0 ? ++_nextId : layer;

        // skip 0 as it is special
        if (id == 0)
            id = kAmplimixLayersCount;

        AmplimixMutexLocker lock(this);

        // get layer for next sound handle id
        auto* lay = GetLayer(layer);

        // check if corresponding layer is free
        if (AMPLIMIX_LOAD(&lay->flag) == ePSF_MIN)
        {
            // fill in non-atomic layer data along with truncating start and end
            lay->id = id;
            lay->snd = sound;

#if defined(AM_SIMD_INTRINSICS)
            lay->start = startFrame & ~(kProcessedFramesCount - 1);
            lay->end = endFrame & ~(kProcessedFramesCount - 1);
#else
            lay->start = startFrame;
            lay->end = endFrame;
#endif // AM_SIMD_INTRINSICS

            // convert gain and pan to left and right gain and store it atomically
            AMPLIMIX_STORE(&lay->gain, LRGain(gain, pan));
            // store the pitch
            AMPLIMIX_STORE(&lay->pitch, pitch);
            // store the playback speed
            AMPLIMIX_STORE(&lay->userPlaySpeed, speed);
            // atomically set cursor to start position based on given argument
            AMPLIMIX_STORE(&lay->cursor, lay->start);
            // store the base sample rate ratio for this source
            AMPLIMIX_STORE(
                &lay->baseSampleRateRatio,
                static_cast<AmReal32>(sound->format.GetSampleRate()) / static_cast<AmReal32>(_device.mRequestedOutputSampleRate));

            // Initialize the converter
            lay->dataConverter = ampoolnew(MemoryPoolKind::Amplimix, AudioConverter);

            const auto soundChannels = static_cast<AmUInt32>(sound->format.GetNumChannels());
            const auto reqChannels = static_cast<AmUInt32>(_device.mRequestedOutputChannels);

            const AmUInt32 soundSampleRate = sound->format.GetSampleRate();
            const AmUInt32 reqSampleRate = _device.mRequestedOutputSampleRate;

            AudioConverter::Settings converterSettings{};
            converterSettings.m_sourceChannelCount = soundChannels;
            converterSettings.m_targetChannelCount = reqChannels;
            converterSettings.m_sourceSampleRate = soundSampleRate;
            converterSettings.m_targetSampleRate = reqSampleRate;

            if (!lay->dataConverter->Configure(converterSettings))
            {
                amLogError("Cannot process frames. Unable to initialize the samples data converter.");
                return 0;
            }

            // store flag last, releasing the layer to the mixer thread
            AMPLIMIX_STORE(&lay->flag, flag);
            OnSoundStarted(this, lay);
        }

        return layer;
    }

    bool AmplimixImpl::SetObstruction(AmUInt32 id, AmUInt32 layer, AmReal32 obstruction)
    {
        auto* lay = GetLayer(layer);

        // check id and state flag to make sure the id is valid
        if (id != lay->id || AMPLIMIX_LOAD(&lay->flag) <= ePSF_STOP)
        {
            // return failure
            return false;
        }

        // store the obstruction factor in the layer
        AMPLIMIX_STORE(&lay->obstruction, obstruction);

        // return success
        return true;
    }

    bool AmplimixImpl::SetOcclusion(AmUInt32 id, AmUInt32 layer, AmReal32 occlusion)
    {
        auto* lay = GetLayer(layer);

        // check id and state flag to make sure the id is valid
        if (id != lay->id || AMPLIMIX_LOAD(&lay->flag) <= ePSF_STOP)
        {
            // return failure
            return false;
        }

        // store the occlusion factor in the layer
        AMPLIMIX_STORE(&lay->occlusion, occlusion);

        // return success
        return true;
    }

    bool AmplimixImpl::SetGainPan(AmUInt32 id, AmUInt32 layer, AmReal32 gain, AmReal32 pan)
    {
        auto* lay = GetLayer(layer);

        // check id and state flag to make sure the id is valid
        if (id != lay->id || AMPLIMIX_LOAD(&lay->flag) <= ePSF_STOP)
        {
            // return failure
            return false;
        }

        if (lay->snd != nullptr && lay->snd->format.GetNumChannels() == 1)
            pan = 0.0f;

        // convert gain and pan to left and right gain and store it atomically
        AMPLIMIX_STORE(&lay->gain, LRGain(gain, pan));

        // return success
        return true;
    }

    bool AmplimixImpl::SetPitch(AmUInt32 id, AmUInt32 layer, AmReal32 pitch)
    {
        auto* lay = GetLayer(layer);

        // check id and state flag to make sure the id is valid
        if ((id == lay->id) && (AMPLIMIX_LOAD(&lay->flag) > ePSF_STOP))
        {
            // store the pitch value atomically
            AMPLIMIX_STORE(&lay->pitch, pitch);
            // return success
            return true;
        }

        // return failure
        return false;
    }

    bool AmplimixImpl::SetCursor(AmUInt32 id, AmUInt32 layer, AmUInt64 cursor)
    {
        auto* lay = GetLayer(layer);

        // check id and state flag to make sure the id is valid
        if ((id == lay->id) && (AMPLIMIX_LOAD(&lay->flag) > ePSF_STOP))
        {
#if defined(AM_SIMD_INTRINSICS)
            // clamp cursor and truncate to multiple of 16 before storing
            AMPLIMIX_STORE(&lay->cursor, AM_CLAMP(cursor, lay->start, lay->end) & ~(kProcessedFramesCount - 1));
#else
            // clamp cursor and store it
            AMPLIMIX_STORE(&lay->cursor, AM_CLAMP(cursor, lay->start, lay->end));
#endif // AM_SIMD_INTRINSICS

            // return success
            return true;
        }

        // return failure
        return false;
    }

    bool AmplimixImpl::SetPlayState(AmUInt32 id, AmUInt32 layer, PlayStateFlag flag)
    {
        // return failure if given flag invalid
        if (flag >= ePSF_MAX)
            return false;

        AmplimixMutexLocker lock(this);

        // get layer based on the lowest bits of id
        auto* lay = GetLayer(layer);

        // check id and state flag to make sure the id is valid
        if (PlayStateFlag prev; (id == lay->id) && ((prev = AMPLIMIX_LOAD(&lay->flag)) >= ePSF_STOP))
        {
            // return failure if already in desired state
            if (prev == flag)
                return false;

            // run appropriate callback
            if (prev == ePSF_STOP && (flag == ePSF_PLAY || flag == ePSF_LOOP))
                OnSoundStarted(this, lay);
            else if ((prev == ePSF_PLAY || prev == ePSF_LOOP) && flag == ePSF_HALT)
                OnSoundPaused(this, lay);
            else if (prev == ePSF_HALT && (flag == ePSF_PLAY || flag == ePSF_LOOP))
                OnSoundResumed(this, lay);
            else if (prev != ePSF_STOP && flag == ePSF_STOP)
                OnSoundStopped(this, lay);

            // swap if flag has not changed and return if successful
            if (AMPLIMIX_CSWAP(&lay->flag, &prev, flag))
            {
                return true;
            }
        }

        // return failure
        return false;
    }

    PlayStateFlag AmplimixImpl::GetPlayState(AmUInt32 id, AmUInt32 layer)
    {
        // get layer based on the lowest bits of id
        const auto* lay = GetLayer(layer);

        // check id and state flag to make sure the id is valid
        if (PlayStateFlag flag; (id == lay->id) && ((flag = AMPLIMIX_LOAD(&lay->flag)) > ePSF_STOP))
        {
            // return the found flag
            return flag;
        }

        // return failure
        return ePSF_MIN;
    }

    bool AmplimixImpl::SetPlaySpeed(AmUInt32 id, AmUInt32 layer, AmReal32 speed)
    {
        auto* lay = GetLayer(layer);

        // check id and state flag to make sure the id is valid
        if ((id == lay->id) && (AMPLIMIX_LOAD(&lay->flag) > ePSF_STOP))
        {
            // convert gain and pan to left and right gain and store it atomically
            AMPLIMIX_STORE(&lay->userPlaySpeed, speed);
            // return success
            return true;
        }

        // return failure
        return false;
    }

    void AmplimixImpl::SetMasterGain(AmReal32 gain)
    {
        AMPLIMIX_STORE(&_masterGain, gain);
    }

    void AmplimixImpl::StopAll()
    {
        AmplimixMutexLocker lock(this);

        // go through all active layers and set their states to the stop state
        for (auto&& lay : _layers)
        {
            // check if active and set to stop if true
            if (AMPLIMIX_LOAD(&lay.flag) > ePSF_STOP)
                AMPLIMIX_STORE(&lay.flag, ePSF_STOP);
        }
    }

    void AmplimixImpl::HaltAll()
    {
        AmplimixMutexLocker lock(this);

        // go through all playing layers and set their states to halt
        for (auto&& lay : _layers)
        {
            // check if playing or looping and try to swap
            if (PlayStateFlag flag; (flag = AMPLIMIX_LOAD(&lay.flag)) > ePSF_HALT)
                AMPLIMIX_CSWAP(&lay.flag, &flag, ePSF_HALT);
        }
    }

    void AmplimixImpl::PlayAll()
    {
        AmplimixMutexLocker lock(this);

        // go through all halted layers and set their states to play
        for (auto&& lay : _layers)
        {
            // need to reset each time
            PlayStateFlag flag = ePSF_HALT;
            // swap the flag to play if it is on halt
            AMPLIMIX_CSWAP(&lay.flag, &flag, ePSF_PLAY);
        }
    }

    bool AmplimixImpl::IsInsideThreadMutex() const
    {
        if (const AmUInt64 threadId = Thread::GetCurrentThreadId(); _insideAudioThreadMutex.contains(threadId))
            return _insideAudioThreadMutex.at(threadId);

        return false;
    }

    void AmplimixImpl::PushCommand(const MixerCommand& command)
    {
        _commandsStack.push(command);
    }

    const Pipeline* AmplimixImpl::GetPipeline() const
    {
        return _pipeline.get();
    }

    Pipeline* AmplimixImpl::GetPipeline()
    {
        return _pipeline.get();
    }

    void AmplimixImpl::IncrementSoundLoopCount(SoundInstance* sound)
    {
        ++sound->_currentLoopCount;
    }

    void AmplimixImpl::ExecuteCommands()
    {
        while (!_commandsStack.empty())
        {
            if (const auto& command = _commandsStack.front(); command.callback)
                AM_UNUSED(command.callback());

            _commandsStack.pop();
        }
    }

    void AmplimixImpl::MixLayer(AmplimixLayerImpl* layer, AudioBuffer* buffer, AmUInt64 frameCount)
    {
        if (layer->snd == nullptr)
        {
            AMPLITUDE_ASSERT(false); // This should technically never appear
            return;
        }

        if (_pipeline == nullptr)
        {
            amLogWarning("No active pipeline is set, this means no sound will be rendered. You should configure the Amplimix "
                         "pipeline in your engine configuration file.");
            return;
        }

        const auto reqChannels = static_cast<AmUInt16>(_device.mRequestedOutputChannels);

        // load flag value atomically first
        PlayStateFlag flag = AMPLIMIX_LOAD(&layer->flag);

        // atomically load cursor
        AmUInt64 cursor = AMPLIMIX_LOAD(&layer->cursor);

        // atomically load left and right gain
        const AmVec2 g = AMPLIMIX_LOAD(&layer->gain);
        const AmReal32 gain = AMPLIMIX_LOAD(&_masterGain);

        AmReal32 l = g.X, r = g.Y;
        if (_device.mRequestedOutputChannels == PlaybackOutputChannels::Mono)
            l = r = (l + r) * AM_InvSqrtF(2);

#if defined(AM_SIMD_INTRINSICS)
        auto lGain = xsimd::batch(l * gain);
        auto rGain = xsimd::batch(r * gain);
#else
        const auto lGain = l * gain;
        const auto rGain = r * gain;
#endif // AM_SIMD_INTRINSICS

        // loop state
        const bool loop = flag == ePSF_LOOP;

        const AmUInt16 soundChannels = layer->snd->format.GetNumChannels();
        const AmReal32 sampleRateRatio = AMPLIMIX_LOAD(&layer->sampleRateRatio);

        AmUInt64 outSamples = frameCount;
        AmUInt64 inSamples = frameCount;

        if (sampleRateRatio != 1.0f)
            inSamples = layer->dataConverter->GetRequiredInputFrameCount(outSamples) - layer->dataConverter->GetInputLatency();

#if defined(AM_SIMD_INTRINSICS)
        inSamples = AM_VALUE_ALIGN(inSamples, kProcessedFramesCount);
#endif // AM_SIMD_INTRINSICS

        SoundChunk* in = SoundChunk::CreateChunk(inSamples, soundChannels, MemoryPoolKind::Amplimix);
        SoundChunk* out = SoundChunk::CreateChunk(AM_MAX(inSamples, outSamples), reqChannels, MemoryPoolKind::Amplimix);

        // if this sound is streaming, and we have a stream event callback
        if (layer->snd->stream)
        {
            // mix sound per chunk of streamed data
            AmUInt64 c = inSamples;
            while (c > 0 && flag != ePSF_MIN)
            {
                // update flag value
                flag = AMPLIMIX_LOAD(&layer->flag);

                if (flag == ePSF_MIN)
                    break;

                const AmUInt64 chunkSize = AM_MIN(layer->snd->chunk->frames, c);
                /* */ AmUInt64 readLen = chunkSize;

#if defined(AM_SIMD_INTRINSICS)
                readLen = AM_VALUE_ALIGN(readLen, kProcessedFramesCount);
#endif // AM_SIMD_INTRINSICS

                readLen = OnSoundStream(this, layer, (cursor + (inSamples - c)) % layer->snd->length, readLen);
                readLen = AM_MIN(readLen, chunkSize);

                // having 0 here mainly means that we have reached
                // the end of the stream and the audio is not looping.
                if (readLen == 0)
                    break;

                AudioBuffer::Copy(*layer->snd->chunk->buffer, 0, *in->buffer, inSamples - c, readLen);

                c -= readLen;
            }
        }
        else
        {
            // Compute offset
            const AmUInt64 offset = cursor % layer->snd->length;
            const AmUInt64 remaining = layer->snd->chunk->frames - cursor;

            if (cursor < layer->snd->chunk->frames && remaining < inSamples)
            {
                AudioBuffer::Copy(*layer->snd->chunk->buffer, offset, *in->buffer, 0, remaining);
                AudioBuffer::Copy(*layer->snd->chunk->buffer, 0, *in->buffer, remaining, in->frames - remaining);
            }
            else
            {
                AudioBuffer::Copy(*layer->snd->chunk->buffer, offset, *in->buffer, 0, in->frames);
            }
        }

        layer->dataConverter->Process(*in->buffer, inSamples, *out->buffer, outSamples);

        if (outSamples > 0 && flag >= ePSF_PLAY)
        {
            // Cache cursor
            AmUInt64 oldCursor = cursor;

            // Execute Pipeline
            // _pipeline->Process(layer, *out->buffer, *out->buffer);
            _pipeline->Execute(layer, *out->buffer, *out->buffer);

            /* */ AmReal32 position = cursor;
            const AmUInt64 start = layer->start;
            const AmUInt64 end = layer->end;

            const auto step = static_cast<AmReal32>(inSamples) / static_cast<AmReal32>(outSamples);

            // regular playback
            for (AmUInt64 i = 0; i < outSamples; i += kProcessedFramesCount)
            {
                position = AM_CLAMP(position, start, end);

                // check if cursor at end
                if (std::ceil(position) == end)
                {
                    // quit unless looping
                    if (!loop)
                        break;

                    // call the onLoop callback
                    if (OnSoundLooped(this, layer))
                    {
                        // wrap around if allowed looping again
                        position = start;
                    }
                    else
                    {
                        // reset data converter
                        layer->dataConverter->Reset();

                        // stop playback
                        break;
                    }
                }

                switch (_device.mRequestedOutputChannels)
                {
                case PlaybackOutputChannels::Mono:
                    // lGain is always equal to rGain on mono
                    MixMono(i, lGain, out->buffer->GetChannel(0), buffer->GetChannel(0));
                    break;

                case PlaybackOutputChannels::Stereo:
                    MixMono(i, lGain, out->buffer->GetChannel(0), buffer->GetChannel(0));
                    MixMono(i, rGain, out->buffer->GetChannel(1), buffer->GetChannel(1));
                    break;

                default:
                    amLogWarning("The mixer cannot handle the requested output channels.");
                    break;
                }

                position += step * kProcessedFramesCount;
            }

            cursor += inSamples;

#if defined(AM_SIMD_INTRINSICS)
            // cursor = AM_VALUE_ALIGN(cursor, kProcessedFramesCount);
#endif // AM_SIMD_INTRINSICS

            cursor = AM_CLAMP(cursor, layer->start, layer->end);

            // swap back cursor if unchanged
            if (!AMPLIMIX_CSWAP(&layer->cursor, &oldCursor, cursor))
                cursor = oldCursor;
        }

        SoundChunk::DestroyChunk(out);
        SoundChunk::DestroyChunk(in);

        // run callback if reached the end
        if (cursor == layer->end)
        {
            // We are in the audio thread mutex here
            const MixerCommandCallback callback = [this, layer, loop]() -> bool
            {
                // stop playback unless looping
                if (!loop)
                {
                    OnSoundEnded(this, layer);
                }
                else
                {
                    // call the onLoop callback
                    if (ShouldLoopSound(this, layer))
                    {
                        // wrap around if allowed looping again
                        AMPLIMIX_CSWAP(&layer->cursor, &layer->end, layer->start);
                    }
                    else
                    {
                        // stop playback
                        OnSoundEnded(this, layer);
                    }
                }

                return true;
            };

            // Postpone call outside the audio thread mutex
            PushCommand({ callback });
        }
    }

    AmplimixLayerImpl* AmplimixImpl::GetLayer(AmUInt32 layer)
    {
        // get layer based on the lowest bits of layer id
        return &_layers[layer & kAmplimixLayersMask];
    }

    bool AmplimixImpl::ShouldMix(AmplimixLayerImpl* layer)
    {
        if (layer->snd == nullptr)
            return false;

        // load flag value
        PlayStateFlag flag = AMPLIMIX_LOAD(&layer->flag);

        // return if flag is not cleared
        return (flag > ePSF_HALT);
    }

    void AmplimixImpl::UpdatePitch(AmplimixLayerImpl* layer)
    {
        const AmReal32 pitch = AMPLIMIX_LOAD(&layer->pitch);
        const AmReal32 speed = AMPLIMIX_LOAD(&layer->userPlaySpeed);

        /* */ AmReal32 currentSpeed = AMPLIMIX_LOAD(&layer->playSpeed);
        const AmReal32 playSpeed = AM_MAX(pitch * speed, 0.001f);

        if (currentSpeed != playSpeed)
        {
            currentSpeed = AM_Lerp(currentSpeed, 0.75f, playSpeed);

            const AmReal32 baseSampleRateRatio = AMPLIMIX_LOAD(&layer->baseSampleRateRatio);
            const AmReal32 sampleRateRatio = baseSampleRateRatio * currentSpeed;

            AMPLIMIX_STORE(&layer->targetPlaySpeed, playSpeed);
            AMPLIMIX_STORE(&layer->sampleRateRatio, sampleRateRatio);

            const AmUInt64 t = 1000;
            const AmUInt64 s = (AmUInt64)(sampleRateRatio * t);

            AMPLITUDE_ASSERT(s != 0);
            layer->dataConverter->SetSampleRate(s, t);

            AMPLIMIX_STORE(&layer->playSpeed, currentSpeed);
        }
    }

    void AmplimixImpl::LockAudioMutex()
    {
        if (_audioThreadMutex)
            Thread::LockMutex(_audioThreadMutex);

        _insideAudioThreadMutex[Thread::GetCurrentThreadId()] = true;
    }

    void AmplimixImpl::UnlockAudioMutex()
    {
        AMPLITUDE_ASSERT(IsInsideThreadMutex());

        if (_audioThreadMutex)
            Thread::UnlockMutex(_audioThreadMutex);

        _insideAudioThreadMutex[Thread::GetCurrentThreadId()] = false;
    }

    void AmplimixLayerImpl::Reset()
    {
        ampooldelete(MemoryPoolKind::Amplimix, AudioConverter, dataConverter);
    }

    AmUInt32 AmplimixLayerImpl::GetId() const
    {
        return id;
    }

    AmUInt64 AmplimixLayerImpl::GetStartPosition() const
    {
        return start;
    }

    AmUInt64 AmplimixLayerImpl::GetEndPosition() const
    {
        return end;
    }

    AmUInt64 AmplimixLayerImpl::GetCurrentPosition() const
    {
        return AMPLIMIX_LOAD(&cursor);
    }

    AmVec2 AmplimixLayerImpl::GetGain() const
    {
        return AMPLIMIX_LOAD(&gain);
    }

    AmReal32 AmplimixLayerImpl::GetPitch() const
    {
        return AMPLIMIX_LOAD(&pitch);
    }

    AmReal32 AmplimixLayerImpl::GetObstruction() const
    {
        return AMPLIMIX_LOAD(&obstruction);
    }

    AmReal32 AmplimixLayerImpl::GetOcclusion() const
    {
        return AMPLIMIX_LOAD(&occlusion);
    }

    AmReal32 AmplimixLayerImpl::GetPlaySpeed() const
    {
        return AMPLIMIX_LOAD(&playSpeed);
    }

    AmVec3 AmplimixLayerImpl::GetLocation() const
    {
        if (snd == nullptr || snd->sound == nullptr)
            return AM_V3(0.0f, 0.0f, 0.0f);

        return snd->sound->GetChannel().GetLocation();
    }

    Entity AmplimixLayerImpl::GetEntity() const
    {
        if (snd == nullptr || snd->sound == nullptr)
            return Entity(nullptr);

        return snd->sound->GetChannel().GetEntity();
    }

    Listener AmplimixLayerImpl::GetListener() const
    {
        if (snd == nullptr || snd->sound == nullptr)
            return Listener(nullptr);

        return snd->sound->GetChannel().GetListener();
    }

    Channel AmplimixLayerImpl::GetChannel() const
    {
        if (snd == nullptr || snd->sound == nullptr)
            return Channel(nullptr);

        return snd->sound->GetChannel();
    }

    SoundFormat AmplimixLayerImpl::GetSoundFormat() const
    {
        if (snd == nullptr || snd->sound == nullptr)
            return SoundFormat();

        return snd->format;
    }

    eSpatialization AmplimixLayerImpl::GetSpatialization() const
    {
        if (snd == nullptr || snd->sound == nullptr)
            return eSpatialization_None;

        return static_cast<eSpatialization>(snd->sound->GetSettings().m_spatialization);
    }

    bool AmplimixLayerImpl::IsLoopEnabled() const
    {
        if (snd == nullptr || snd->sound == nullptr)
            return false;

        return snd->sound->GetSettings().m_loop;
    }

    bool AmplimixLayerImpl::IsStreamEnabled() const
    {
        if (snd == nullptr || snd->sound == nullptr)
            return false;

        return snd->sound->GetSound()->IsStream();
    }

    const Sound* AmplimixLayerImpl::GetSound() const
    {
        if (snd == nullptr || snd->sound == nullptr)
            return nullptr;

        return snd->sound->GetSound();
    }

    const EffectInstance* AmplimixLayerImpl::GetEffect() const
    {
        if (snd == nullptr || snd->sound == nullptr)
            return nullptr;

        return snd->sound->GetEffect();
    }

    AmUInt32 AmplimixLayerImpl::GetSampleRate() const
    {
        if (snd == nullptr || snd->sound == nullptr)
            return 0;

        const AmReal32 ratio = AMPLIMIX_LOAD(&sampleRateRatio);
        return snd->format.GetSampleRate() * ratio;
    }
} // namespace SparkyStudios::Audio::Amplitude
