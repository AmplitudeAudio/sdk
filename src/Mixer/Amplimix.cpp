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

#include <thread>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Core/Engine.h>
#include <Core/Playback/ChannelInternalState.h>
#include <Mixer/Amplimix.h>
#include <Mixer/Pipeline.h>

#define AMPLIMIX_STORE(A, C) std::atomic_store_explicit(A, (C), std::memory_order_release)
#define AMPLIMIX_LOAD(A) std::atomic_load_explicit(A, std::memory_order_acquire)
#define AMPLIMIX_CSWAP(A, E, C) std::atomic_compare_exchange_strong_explicit(A, E, C, std::memory_order_acq_rel, std::memory_order_acquire)

// Relaxed load/store for independently meaningful values (gain, cursor, etc.)
// where no ordering with other memory operations is required.
#define AMPLIMIX_LOAD_RELAXED(A) std::atomic_load_explicit(A, std::memory_order_relaxed)
#define AMPLIMIX_STORE_RELAXED(A, C) std::atomic_store_explicit(A, (C), std::memory_order_relaxed)

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
        AmplimixImpl* m_mixer = nullptr;
        bool m_locked = false;
    };

    constexpr AmUInt32 kProcessedFramesCount = GetSimdBlockSize();

    static void OnSoundDestroyed(AmplimixImpl* mixer, AmplimixLayerImpl* layer);

    static void TriggerChannelEvents(ChannelInternalState* channelState, eChannelEvent event)
    {
        amEngine->OnNextFrame(
            [channelState, event](AmTime)
            {
                channelState->Trigger(event);
            });
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
        amLogDebug("Started sound: '%s'.", sound->GetSound()->GetName().c_str());

        const auto channel = sound->GetChannel();
        auto* channelState = channel.GetState();

        TriggerChannelEvents(channelState, eChannelEvent_Begin);
    }

    static void OnSoundPaused(AmplimixImpl* mixer, AmplimixLayerImpl* layer)
    {
        const auto* sound = layer->snd->sound.get();
        amLogDebug("Paused sound: '%s'.", sound->GetSound()->GetName().c_str());

        const auto channel = sound->GetChannel();
        auto* channelState = channel.GetState();

        TriggerChannelEvents(channelState, eChannelEvent_Pause);
    }

    static void OnSoundResumed(AmplimixImpl* mixer, AmplimixLayerImpl* layer)
    {
        const auto* sound = layer->snd->sound.get();
        amLogDebug("Resumed sound: '%s'.", sound->GetSound()->GetName().c_str());

        const auto channel = sound->GetChannel();
        auto* channelState = channel.GetState();

        TriggerChannelEvents(channelState, eChannelEvent_Resume);
    }

    static void OnSoundStopped(AmplimixImpl* mixer, AmplimixLayerImpl* layer)
    {
        const auto* sound = layer->snd->sound.get();
        amLogDebug("Stopped sound: '%s'.", sound->GetSound()->GetName().c_str());

        const auto channel = sound->GetChannel();
        auto* channelState = channel.GetState();

        TriggerChannelEvents(channelState, eChannelEvent_Stop);
    }

    static bool OnSoundLooped(AmplimixImpl* mixer, AmplimixLayerImpl* layer)
    {
        auto* sound = layer->snd->sound.get();
        amLogDebug("Looped sound: '%s'.", sound->GetSound()->GetName().c_str());

        AmplimixImpl::IncrementSoundLoopCount(sound);

        const bool shouldLoop = ShouldLoopSound(mixer, layer);

        if (shouldLoop)
        {
            const auto channel = sound->GetChannel();
            auto* channelState = channel.GetState();

            TriggerChannelEvents(channelState, eChannelEvent_Loop);
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
        amLogDebug("Ended sound: '%s'.", sound->GetSound()->GetName().c_str());

        const auto channel = sound->GetChannel();
        auto* channelState = channel.GetState();

        auto haltCallback = [channelState]() -> bool
        {
            TriggerChannelEvents(channelState, eChannelEvent_End);
            channelState->HaltInternal();

            return true;
        };

        if (const auto* engine = static_cast<const EngineImpl*>(Engine::GetInstance()); engine->GetState()->stopping)
        {
            mixer->PushCommand({ haltCallback });
            return;
        }

        if (sound->GetSettings().m_kind == SoundKind::Standalone)
        {
            mixer->PushCommand({ haltCallback });
        }
        else if (sound->GetSettings().m_kind == SoundKind::Switched)
        {
            mixer->PushCommand({ haltCallback });
        }
        else if (sound->GetSettings().m_kind == SoundKind::Contained)
        {
            const CollectionImpl* collection = sound->GetCollection();
            AMPLITUDE_ASSERT(collection != nullptr); // Should always have a collection for contained sound instances.

            if (const CollectionDefinition* config = collection->GetDefinition(); config->play_mode() == CollectionPlayMode_PlayAll)
            {
                amEngine->OnNextFrame(
                    [collection, channelState, sound, mixer, haltCallback](AmTime)
                    {
                        channelState->GetRealChannel().MarkAsPlayed(sound->GetSound());
                        if (channelState->GetRealChannel().AllSoundsHasPlayed())
                        {
                            channelState->GetRealChannel().ClearPlayedSounds();
                            mixer->PushCommand({ haltCallback });
                        }

                        // Play the collection again only if the channel is still playing.
                        if (channelState->GetRealChannel().Playing())
                            channelState->Play();
                    });
            }
        }
        else
        {
            AMPLITUDE_ASSERT(false); // Should never fall in this case.
        }
    }

    static void OnSoundDestroyed(AmplimixImpl* mixer, AmplimixLayerImpl* layer)
    {
        // Null guard: multiple deferred commands may target the same layer in one
        // ExecuteCommands() pass (e.g., HaltInternal queues via SetPlayState + OnSoundEnded
        // queues another). The first call nulls snd via Destroy(); subsequent calls are no-ops.
        if (layer->snd == nullptr)
            return;

        mixer->DeactivateLayer(mixer->GetLayerIndex(layer));
        layer->Destroy();
    }

    static void MixMono(AmUInt64 index, const simd_batch& gain, const AudioBufferChannel& in, AudioBufferChannel& out)
    {
#if defined(AM_SIMD_INTRINSICS)
        const auto x = xsimd::load_aligned<simd_arch>(&in[index]);
        const auto y = xsimd::load_aligned<simd_arch>(&out[index]);

        xsimd::store_aligned<simd_arch>(&out[index], xsimd::fma(x, gain, y));
#else
        out[index] += in[index] * gain;
#endif // AM_SIMD_INTRINSICS
    }

    AmplimixImpl::AmplimixImpl(AmReal32 masterGain)
        : _initialized(false)
        , _commandsStack()
        , _audioThreadMutex()
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

        _pipeline = Engine::GetInstance()->GetPipelineHandle();

        if (_pipeline == nullptr)
        {
            amLogCritical("Invalid pipeline configuration.");
            return false;
        }

        _engineState = amEngine->GetState().get();

        _device.mOutputBufferSize = config->output()->buffer_size();
        _device.mRequestedOutputSampleRate = config->output()->frequency();
        _device.mRequestedOutputChannels = PlaybackOutputChannels::Stereo; // For now, only support stereo output.
        _device.mRequestedOutputFormat = static_cast<PlaybackOutputFormat>(config->output()->format());

        // Store the resampler name
        if (config->mixer()->resampler() != nullptr && !config->mixer()->resampler()->empty())
            _resamplerName = config->mixer()->resampler()->str();
        else
            _resamplerName = "default";

        // Publish to atomic snapshots for audio-thread reads
        AMPLIMIX_STORE_RELAXED(&_mixOutputSampleRate, _device.mRequestedOutputSampleRate);
        AMPLIMIX_STORE_RELAXED(&_mixOutputChannels, _device.mRequestedOutputChannels);

        _initialized = true;

        return true;
    }

    void AmplimixImpl::Deinit()
    {
        if (!_initialized)
            return;

        AMPLITUDE_ASSERT(!IsInsideThreadMutex());

        // Drain any pending deferred commands
        ExecuteCommands();

        for (auto& layer : _layers)
            layer.Destroy();

        _initialized = false;
        _pipeline = nullptr;
        _engineState = nullptr;

        AMPLIMIX_STORE_RELAXED(&_activeLayerCount, 0);
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

        // Publish to atomic snapshots for audio-thread reads
        AMPLIMIX_STORE_RELAXED(&_mixOutputSampleRate, _device.mRequestedOutputSampleRate);
        AMPLIMIX_STORE_RELAXED(&_mixOutputChannels, _device.mRequestedOutputChannels);
    }

    void AmplimixImpl::SetAfterMixCallback(AfterMixCallback callback)
    {
        _afterMixCallback = callback;
    }

    AmUInt64 AmplimixImpl::Mix(AudioBuffer** outBuffer, AmUInt64 frameCount)
    {
        if (outBuffer != nullptr)
            // Enforce the output buffer to be null before calling Mix
            *outBuffer = nullptr;

        const bool stopping = _engineState != nullptr && _engineState->stopping.load(std::memory_order_acquire);
        const bool paused = _engineState != nullptr && _engineState->paused.load(std::memory_order_acquire);

        if (!_initialized || _engineState == nullptr || stopping || paused)
            return 0;

        // Mark that we're now mixing
        _isMixing.store(true, std::memory_order_release);

        // RAII guard to clear flag on exit
        struct MixGuard
        {
            AmplimixImpl* self;
            ~MixGuard()
            {
                {
                    std::lock_guard<std::mutex> lock(self->_mixCompleteMutex);
                    self->_isMixing.store(false, std::memory_order_release);
                }
                self->_mixCompleteCV.notify_all();
            }
        } mixGuard{ this };

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
        const AmUInt32 activeLayerCount = AMPLIMIX_LOAD_RELAXED(&_activeLayerCount);

        for (AmUInt32 i = 0; i < activeLayerCount; ++i)
        {
            if (_engineState->stopping.load(std::memory_order_relaxed))
                break; // Stop mixing if engine is stopping

            auto& layer = _layers[_activeLayerIndices[i]];

            if (!ShouldMix(&layer))
                continue;

            UpdatePitch(&layer);

            // Update cached instance data for multi-position processing
            layer.UpdateInstanceData();

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

            layer.ResetPipeline();
        }

        ExecuteCommands();

        if (hasMixedAtLeastOneLayer)
        {
            // Run the after-mix callback if available
            if (_afterMixCallback != nullptr)
                _afterMixCallback(this, &_scratchBuffer, frameCount);

            if (outBuffer != nullptr)
                *outBuffer = &_scratchBuffer;

            return frameCount;
        }

        return 0;
    }

    AmUInt32 AmplimixImpl::Play(
        SoundData* sound, PlayStateFlag flag, AmReal32 gain, AmReal32 pitch, AmReal32 speed, AmUInt32 id, AmUInt32 layer)
    {
        return PlayAdvanced(sound, flag, gain, pitch, speed, 0, sound->length, id, layer);
    }

    AmUInt32 AmplimixImpl::PlayAdvanced(
        SoundData* sound,
        PlayStateFlag flag,
        AmReal32 gain,
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

        AmplimixMutexLocker lock(this);

        // define a layer id
        layer = layer == 0 ? ++_nextId : layer;

        // skip 0 as it is special
        if (id == 0)
            id = kAmplimixLayersCount;

        // get layer for next sound handle id
        auto* lay = GetLayer(layer);

        // check if corresponding layer is free
        if (AMPLIMIX_LOAD(&lay->flag) == ePSF_MIN)
        {
            // Initialize this layer's pipeline
            lay->pipeline = _pipeline->CreateInstance(lay);

            // All non-atomic writes (id, snd, start, end) happen before the release-store
            // to lay->flag below. The audio thread's acquire-load on flag in ShouldMix()
            // establishes happens-before, guaranteeing these fields are visible.
            lay->id = id;
            lay->snd = sound;

#if defined(AM_SIMD_INTRINSICS)
            lay->start = startFrame & ~(kProcessedFramesCount - 1);
            lay->end = endFrame & ~(kProcessedFramesCount - 1);
#else
            lay->start = startFrame;
            lay->end = endFrame;
#endif // AM_SIMD_INTRINSICS

            // store the gain
            AMPLIMIX_STORE(&lay->gain, gain);
            // store the pitch
            AMPLIMIX_STORE(&lay->pitch, pitch);
            // store the playback speed
            AMPLIMIX_STORE(&lay->userPlaySpeed, speed);
            // initial value for the current speed
            AMPLIMIX_STORE(&lay->playSpeed, pitch * speed);
            // atomically set cursor to start position based on given argument
            AMPLIMIX_STORE(&lay->cursor, lay->start);

            const AmUInt32 reqSampleRate = AMPLIMIX_LOAD_RELAXED(&_mixOutputSampleRate);
            const AmReal32 baseRatio = static_cast<AmReal32>(sound->format.GetSampleRate()) / static_cast<AmReal32>(reqSampleRate);
            // store the base sample rate ratio for this source
            AMPLIMIX_STORE(&lay->baseSampleRateRatio, baseRatio);
            // store the initial value for sample rate ratio
            AMPLIMIX_STORE(&lay->sampleRateRatio, baseRatio * pitch * speed);

            // Initialize the converter
            lay->dataConverter = ampoolnew(eMemoryPoolKind_Amplimix, AudioConverter, _resamplerName);

            const auto soundChannels = static_cast<AmUInt32>(sound->format.GetNumChannels());
            const AmUInt32 soundSampleRate = sound->format.GetSampleRate();

            AudioConverter::Settings converterSettings{};
            converterSettings.m_sourceChannelCount = soundChannels;
            converterSettings.m_targetChannelCount = 1; // Sound is always processed as mono
            converterSettings.m_sourceSampleRate = soundSampleRate;
            converterSettings.m_targetSampleRate = reqSampleRate;

            if (!lay->dataConverter->Configure(converterSettings))
            {
                amLogError("Cannot process frames. Unable to initialize the samples data converter.");
                return 0;
            }

            // Pre-warm the layer's chunk pool so the audio thread never allocates.
            // mOutputBufferSize is a sample count. A callback
            // larger than the nominal period grows the pool once, then matches
            // thereafter; a smaller one keeps the oversized chunk (Acquire reuses by
            // capacity) and the pipeline auto-configures nodes to the buffer capacity,
            // processing silent padding and advancing effects — a sustained size change
            // re-runs every node's Configure on the audio thread.
            const auto outChannels = static_cast<AmInt16>(_device.mRequestedOutputChannels);
            const AmUInt64 outFrames = _device.mOutputBufferSize / outChannels;
            const AmUInt64 inFrames = lay->dataConverter->GetRequiredInputFrameCount(outFrames);

            lay->_chunkPool.PreWarm(inFrames, static_cast<AmUInt16>(soundChannels), outFrames);

            // store flag last, releasing the layer to the mixer thread
            AMPLIMIX_STORE(&lay->flag, flag);

            PushCommand({ [this, lay]() -> bool
                          {
                              // Add to active layer list
                              ActivateLayer(GetLayerIndex(lay));
                              return true;
                          } });

            OnSoundStarted(this, lay);
        }

        return layer;
    }

    bool AmplimixImpl::SetObstruction(AmUInt32 id, AmUInt32 layer, AmReal32 obstruction)
    {
        auto* lay = GetLayer(layer);

        if (AMPLIMIX_LOAD(&lay->flag) <= ePSF_STOP || id != lay->id)
            return false;

        AMPLIMIX_STORE(&lay->obstruction, obstruction);

        return true;
    }

    bool AmplimixImpl::SetOcclusion(AmUInt32 id, AmUInt32 layer, AmReal32 occlusion)
    {
        auto* lay = GetLayer(layer);

        if (AMPLIMIX_LOAD(&lay->flag) <= ePSF_STOP || id != lay->id)
            return false;

        AMPLIMIX_STORE(&lay->occlusion, occlusion);

        return true;
    }

    bool AmplimixImpl::SetGain(AmUInt32 id, AmUInt32 layer, AmReal32 gain)
    {
        auto* lay = GetLayer(layer);

        if (AMPLIMIX_LOAD(&lay->flag) <= ePSF_STOP || id != lay->id)
            return false;

        AMPLIMIX_STORE(&lay->gain, gain);

        return true;
    }

    bool AmplimixImpl::SetPitch(AmUInt32 id, AmUInt32 layer, AmReal32 pitch)
    {
        auto* lay = GetLayer(layer);

        if (AMPLIMIX_LOAD(&lay->flag) <= ePSF_STOP || id != lay->id)
            return false;

        AMPLIMIX_STORE(&lay->pitch, pitch);

        return true;
    }

    bool AmplimixImpl::SetCursor(AmUInt32 id, AmUInt32 layer, AmUInt64 cursor)
    {
        auto* lay = GetLayer(layer);

        if (AMPLIMIX_LOAD(&lay->flag) <= ePSF_STOP || id != lay->id)
            return false;

#if defined(AM_SIMD_INTRINSICS)
        // clamp cursor and truncate to multiple of kProcessedFramesCount before storing
        AMPLIMIX_STORE(&lay->cursor, AM_CLAMP(cursor, lay->start, lay->end) & ~(kProcessedFramesCount - 1));
#else
        // clamp cursor and store it
        AMPLIMIX_STORE(&lay->cursor, AM_CLAMP(cursor, lay->start, lay->end));
#endif // AM_SIMD_INTRINSICS

        return true;
    }

    bool AmplimixImpl::GetCursor(AmUInt32 id, AmUInt32 layer, AmUInt64& cursor)
    {
        auto* lay = GetLayer(layer);

        if (AMPLIMIX_LOAD(&lay->flag) <= ePSF_STOP || id != lay->id)
            return false;

        cursor = AMPLIMIX_LOAD(&lay->cursor);

        return true;
    }

    bool AmplimixImpl::ResetLayerState(AmUInt32 id, AmUInt32 layer)
    {
        auto* lay = GetLayer(layer);

        if (AMPLIMIX_LOAD(&lay->flag) <= ePSF_STOP || id != lay->id)
            return false;

        if (lay->dataConverter != nullptr)
            lay->dataConverter->Reset();

        lay->ResetPipeline();

        return true;
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
            // return success if already in desired state
            if (prev == flag)
                return true;

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
                if (flag == ePSF_STOP)
                {
                    // Defer sound destruction to the audio thread via command queue.
                    // The CAS to ePSF_STOP prevents ShouldMix() from returning true,
                    // so the audio thread will not access this layer during the next mix cycle.
                    PushCommand({ [this, lay]() -> bool
                                  {
                                      OnSoundDestroyed(this, lay);
                                      return true;
                                  } });
                }

                return true;
            }
        }

        // return failure
        return false;
    }

    PlayStateFlag AmplimixImpl::GetPlayState(AmUInt32 id, AmUInt32 layer)
    {
        // get layer based on the lowest bits of id
        auto* lay = GetLayer(layer);

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

        if (AMPLIMIX_LOAD(&lay->flag) <= ePSF_STOP || id != lay->id)
            return false;

        AMPLIMIX_STORE(&lay->userPlaySpeed, speed);

        return true;
    }

    void AmplimixImpl::SetMasterGain(AmReal32 gain)
    {
        AMPLIMIX_STORE_RELAXED(&_masterGain, gain);
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

    static thread_local bool tl_insideAudioMutex = false;

    bool AmplimixImpl::IsInsideThreadMutex() const
    {
        return tl_insideAudioMutex;
    }

    void AmplimixImpl::PushCommand(const MixerCommand& command)
    {
        // Fast path
        if (_commandsStack.TryEnqueue(command))
            return;

        if (!IsInsideThreadMutex())
        {
            // Spin briefly — the audio thread drains at callback rate (~5ms)
            for (int i = 0; i < 256; ++i)
            {
                std::this_thread::yield();
                if (_commandsStack.TryEnqueue(command))
                    return;
            }

            // Wait for the current mix cycle to finish (drains the queue)
            amLogWarning("Amplimix command queue full, waiting for mix cycle to drain.");
            Wait();

            if (_commandsStack.TryEnqueue(command))
                return;
        }

        amLogWarning("Amplimix command queue still full after wait. Executing command inline (mixer idle).");
        if (command.callback)
            command.callback();
    }

    const Pipeline* AmplimixImpl::GetPipeline() const
    {
        return _pipeline;
    }

    Pipeline* AmplimixImpl::GetPipeline()
    {
        return _pipeline;
    }

    void AmplimixImpl::IncrementSoundLoopCount(SoundInstance* sound)
    {
        ++sound->_currentLoopCount;
    }

    void AmplimixImpl::ExecuteCommands()
    {
        MixerCommand command;
        while (_commandsStack.TryDequeue(command))
            if (command.callback)
                AM_UNUSED(command.callback());
    }

    void AmplimixImpl::MixLayer(AmplimixLayerImpl* layer, AudioBuffer* buffer, AmUInt64 frameCount)
    {
        // snd is guaranteed non-null here: ShouldMix() already verified flag > ePSF_HALT
        // (acquire), and Destroy() (which nulls snd) only runs in ExecuteCommands() after
        // this loop. The assert below is a defense-in-depth check.
        if (layer->snd == nullptr)
        {
            AMPLITUDE_ASSERT(false); // This should technically never appear
            return;
        }

        if (_pipeline == nullptr || layer->pipeline == nullptr)
        {
            amLogWarning("No active pipeline is set, this means no sound will be rendered. You should configure the Amplimix "
                         "pipeline in your engine configuration file.");
            return;
        }

        // Check for separate mode instancing - process each instance independently
        const auto& channel = layer->GetChannel();
        if (channel.Valid() && channel.GetState()->IsInstancingEnabled() &&
            channel.GetState()->GetInstancingMode() == eChannelInstanceMode_Separate && !layer->instanceData.empty())
        {
            MixLayerSeparateMode(layer, buffer, frameCount);
            return;
        }

        // load flag value atomically first
        PlayStateFlag flag = AMPLIMIX_LOAD(&layer->flag);

        // atomically load cursor
        AmUInt64 cursor = AMPLIMIX_LOAD(&layer->cursor);

        // atomically load master gain
        const AmReal32 gain = AMPLIMIX_LOAD(&_masterGain) * AMPLIMIX_LOAD(&layer->gain);

#if defined(AM_SIMD_INTRINSICS)
        auto bGain = simd_batch(gain);
#else
        const auto bGain = gain;
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

        SoundChunk* in = layer->_chunkPool.Acquire(inSamples, soundChannels, false);
        SoundChunk* transient = layer->_chunkPool.Acquire(outSamples, 1);
        SoundChunk* out = layer->_chunkPool.Acquire(transient->frames, 2);

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
                AudioBuffer::Copy(*layer->snd->chunk->buffer, 0, *in->buffer, remaining, inSamples - remaining);
            }
            else
            {
                AudioBuffer::Copy(*layer->snd->chunk->buffer, offset, *in->buffer, 0, inSamples);
            }
        }

        layer->dataConverter->Process(*in->buffer, inSamples, *transient->buffer, outSamples);

        if (outSamples > 0 && flag >= ePSF_PLAY)
        {
            // Cache cursor
            AmUInt64 oldCursor = cursor;

            // Execute Pipeline
            layer->pipeline->Execute(*transient->buffer, *out->buffer);

            /* */ AmReal64 position = cursor;
            const AmUInt64 start = layer->start;
            const AmUInt64 end = layer->end;

            const AmReal64 step = static_cast<AmReal64>(inSamples) / static_cast<AmReal64>(outSamples);

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
                    MixMono(i, bGain, out->buffer->GetChannel(0), buffer->GetChannel(0));
                    break;

                case PlaybackOutputChannels::Stereo:
                    MixMono(i, bGain, out->buffer->GetChannel(0), buffer->GetChannel(0));
                    MixMono(i, bGain, out->buffer->GetChannel(1), buffer->GetChannel(1));
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

        layer->_chunkPool.Release(out);
        layer->_chunkPool.Release(transient);
        layer->_chunkPool.Release(in);

        // run callback if reached the end
        if (cursor == layer->end)
        {
            if (!loop)
            {
                OnSoundEnded(this, layer);
            }
            else
            {
                if (ShouldLoopSound(this, layer))
                    AMPLIMIX_CSWAP(&layer->cursor, &layer->end, layer->start);
                else
                    OnSoundEnded(this, layer);
            }
        }
    }

    void AmplimixImpl::MixLayerSeparateMode(AmplimixLayerImpl* layer, AudioBuffer* buffer, AmUInt64 frameCount)
    {
        if (layer->instanceData.empty())
            return;

        // load flag value atomically first
        PlayStateFlag flag = AMPLIMIX_LOAD(&layer->flag);

        // atomically load master gain
        const AmReal32 gain = AMPLIMIX_LOAD(&_masterGain) * AMPLIMIX_LOAD(&layer->gain);

#if defined(AM_SIMD_INTRINSICS)
        auto bGain = simd_batch(gain);
#else
        const auto bGain = gain;
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

        const AmUInt64 start = layer->start;
        const AmUInt64 end = layer->end;
        bool allInstancesFinished = true;

        // Get the channel for cursor write-back through the atomic slots. The game-owned
        // instance containers are never touched from the audio thread.
        const auto& channel = layer->GetChannel();
        if (!channel.Valid())
            return;

        auto* channelState = channel.GetState();

        // Pre-allocate buffers for instance processing
        SoundChunk* in = layer->_chunkPool.Acquire(inSamples, soundChannels, false);
        SoundChunk* transient = layer->_chunkPool.Acquire(outSamples, 1);
        SoundChunk* out = layer->_chunkPool.Acquire(outSamples, 2);

        // Process each instance
        for (AmSize instanceIndex = 0; instanceIndex < layer->instanceData.size(); ++instanceIndex)
        {
            auto& data = layer->instanceData[instanceIndex];

            if (data.cursor >= end)
                continue;

            allInstancesFinished = false;

            // Set instance context for pipeline nodes
            layer->processingInstance = true;
            layer->currentInstanceIndex = instanceIndex;

            AmUInt64 instanceCursor = data.cursor;

            // Clear buffers for reuse
            in->buffer->Clear();
            transient->buffer->Clear();
            out->buffer->Clear();

            if (layer->snd->stream)
            {
                AmUInt64 c = inSamples;
                while (c > 0 && flag != ePSF_MIN)
                {
                    flag = AMPLIMIX_LOAD(&layer->flag);
                    if (flag == ePSF_MIN)
                        break;

                    const AmUInt64 chunkSize = AM_MIN(layer->snd->chunk->frames, c);
                    AmUInt64 readLen = chunkSize;

#if defined(AM_SIMD_INTRINSICS)
                    readLen = AM_VALUE_ALIGN(readLen, kProcessedFramesCount);
#endif // AM_SIMD_INTRINSICS

                    readLen = OnSoundStream(this, layer, (instanceCursor + (inSamples - c)) % layer->snd->length, readLen);
                    readLen = AM_MIN(readLen, chunkSize);

                    if (readLen == 0)
                        break;

                    AudioBuffer::Copy(*layer->snd->chunk->buffer, 0, *in->buffer, inSamples - c, readLen);
                    c -= readLen;
                }
            }
            else
            {
                const AmUInt64 offset = instanceCursor % layer->snd->length;
                const AmUInt64 remaining = layer->snd->chunk->frames - instanceCursor;

                if (instanceCursor < layer->snd->chunk->frames && remaining < inSamples)
                {
                    AudioBuffer::Copy(*layer->snd->chunk->buffer, offset, *in->buffer, 0, remaining);
                    AudioBuffer::Copy(*layer->snd->chunk->buffer, 0, *in->buffer, remaining, inSamples - remaining);
                }
                else
                {
                    AudioBuffer::Copy(*layer->snd->chunk->buffer, offset, *in->buffer, 0, inSamples);
                }
            }

            // Convert sample rate
            layer->dataConverter->Process(*in->buffer, inSamples, *transient->buffer, outSamples);

            if (outSamples > 0 && flag >= ePSF_PLAY)
            {
                layer->pipeline->Execute(*transient->buffer, *out->buffer);

                AmReal64 position = instanceCursor;
                const AmReal64 step = static_cast<AmReal64>(inSamples) / static_cast<AmReal64>(outSamples);

                // Mix into output buffer
                for (AmUInt64 i = 0; i < outSamples; i += kProcessedFramesCount)
                {
                    position = AM_CLAMP(position, static_cast<AmReal64>(start), static_cast<AmReal64>(end));

                    if (std::ceil(position) == end)
                    {
                        if (loop)
                            position = start;
                        else
                            break;
                    }

                    switch (_device.mRequestedOutputChannels)
                    {
                    case PlaybackOutputChannels::Mono:
                        MixMono(i, bGain, out->buffer->GetChannel(0), buffer->GetChannel(0));
                        break;

                    case PlaybackOutputChannels::Stereo:
                        MixMono(i, bGain, out->buffer->GetChannel(0), buffer->GetChannel(0));
                        MixMono(i, bGain, out->buffer->GetChannel(1), buffer->GetChannel(1));
                        break;

                    default:
                        amLogWarning("The mixer cannot handle the requested output channels.");
                        break;
                    }

                    position += step * kProcessedFramesCount;
                }

                instanceCursor += inSamples;
                instanceCursor = AM_CLAMP(instanceCursor, start, end);
            }

            // Update the instance's cursor in the cached data
            data.cursor = instanceCursor;

            // Write the cursor back through the channel's write-back slots, so the
            // game thread can restore it if this layer is recreated. The id check
            // drops stores that race a re-publication.
            if (auto* slots = channelState->GetInstanceCursorSlots(); slots != nullptr && instanceIndex < kAmMaxChannelInstances)
            {
                auto& slot = slots[instanceIndex];
                if (slot.id.load(std::memory_order_acquire) == data.instanceId)
                    slot.cursor.store(instanceCursor, std::memory_order_release);
            }

            // Reset pipeline state for next instance. This is required since each
            // instance needs to be processed as a single sound object in separate mode.
            layer->ResetPipeline();
        }

        // Clear instance processing context
        layer->processingInstance = false;
        layer->currentInstanceIndex = 0;

        layer->_chunkPool.Release(out);
        layer->_chunkPool.Release(transient);
        layer->_chunkPool.Release(in);

        // If all instances finished, trigger end callback
        if (allInstancesFinished && !loop)
            OnSoundEnded(this, layer);
    }

    AmplimixLayerImpl* AmplimixImpl::GetLayer(AmUInt32 layer)
    {
        // get layer based on the lowest bits of layer id
        return &_layers[layer & kAmplimixLayersMask];
    }

    AmUInt32 AmplimixImpl::GetLayerIndex(const AmplimixLayerImpl* layer) const
    {
        return static_cast<AmUInt32>(layer - _layers);
    }

    void AmplimixImpl::ActivateLayer(AmUInt32 layerIndex)
    {
        AmUInt32 activeLayerCount = AMPLIMIX_LOAD_RELAXED(&_activeLayerCount);
        _activeLayerIndices[activeLayerCount++] = layerIndex;
        AMPLIMIX_STORE_RELAXED(&_activeLayerCount, activeLayerCount);
    }

    void AmplimixImpl::DeactivateLayer(AmUInt32 layerIndex)
    {
        AmUInt32 activeLayerCount = AMPLIMIX_LOAD_RELAXED(&_activeLayerCount);
        for (AmUInt32 i = 0; i < activeLayerCount; ++i)
        {
            if (_activeLayerIndices[i] == layerIndex)
            {
                // Swap with last element for O(1) removal
                _activeLayerIndices[i] = _activeLayerIndices[--activeLayerCount];
                AMPLIMIX_STORE_RELAXED(&_activeLayerCount, activeLayerCount);
                return;
            }
        }
    }

    bool AmplimixImpl::ShouldMix(AmplimixLayerImpl* layer)
    {
        // Acquire-load flag first to establish happens-before with the
        // release-store in PlayAdvanced (which writes snd before setting flag).
        PlayStateFlag flag = AMPLIMIX_LOAD(&layer->flag);

        if (flag <= ePSF_HALT)
            return false;

        // After the acquire on flag, snd is guaranteed non-null because:
        // - PlayAdvanced writes snd before release-storing flag > ePSF_HALT
        // - Destroy() (which nulls snd) only runs in ExecuteCommands() after the mix loop
        AMPLITUDE_ASSERT(layer->snd != nullptr);
        return true;
    }

    void AmplimixImpl::UpdatePitch(AmplimixLayerImpl* layer)
    {
        const AmReal32 pitch = AMPLIMIX_LOAD(&layer->pitch);
        const AmReal32 speed = AMPLIMIX_LOAD(&layer->userPlaySpeed);

        /* */ AmReal32 currentSpeed = AMPLIMIX_LOAD(&layer->playSpeed);
        const AmReal32 playSpeed = AM_MAX(pitch * speed, 0.001f);

        if (currentSpeed != playSpeed)
        {
            currentSpeed = Lerp(0.75f, currentSpeed, playSpeed);

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
        _audioThreadMutex.lock();
        tl_insideAudioMutex = true;
    }

    void AmplimixImpl::UnlockAudioMutex()
    {
        AMPLITUDE_ASSERT(IsInsideThreadMutex());
        tl_insideAudioMutex = false;
        _audioThreadMutex.unlock();
    }

    void AmplimixImpl::Wait()
    {
        std::unique_lock<std::mutex> lock(_mixCompleteMutex);
        const auto timeout = std::chrono::milliseconds(5000);
        if (!_mixCompleteCV.wait_for(
                lock, timeout,
                [this]()
                {
                    return !_isMixing.load(std::memory_order_acquire);
                }))
        {
            amLogWarning("Amplimix::Wait timed out - forcing continue");
        }
    }

    AmplimixLayerImpl::~AmplimixLayerImpl()
    {
        Destroy();
    }

    void AmplimixLayerImpl::Destroy()
    {
        // This method runs only inside ExecuteCommands(), which executes after
        // the mix loop in Mix(). Therefore no audio-thread reads of snd can
        // race with the null assignment below.
        if (dataConverter != nullptr)
        {
            ampooldelete(eMemoryPoolKind_Amplimix, AudioConverter, dataConverter);
            dataConverter = nullptr;
        }

        pipeline = nullptr;

        if (snd != nullptr)
        {
            snd->sound.reset();
            snd = nullptr;
        }

        _chunkPool.Reset();

        AMPLIMIX_STORE(&flag, ePSF_MIN);
    }

    void AmplimixLayerImpl::ResetPipeline()
    {
        // Check if pipeline is valid before resetting
        // The pipeline can be nullptr if the sound is being destroyed on another thread
        if (pipeline != nullptr)
            pipeline->Reset();

        // Clear room update flag to allow re-initialization for next processing pass
        // This is important for per-instance processing in separate mode, where each
        // instance may be in a different room and requires independent room handling
        const Room& room = GetRoom();
        if (room.Valid())
            room.GetState()->SetWasUpdated(false);
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
        return AMPLIMIX_LOAD_RELAXED(&cursor);
    }

    AmReal32 AmplimixLayerImpl::GetGain() const
    {
        return AMPLIMIX_LOAD_RELAXED(&gain);
    }

    AmReal32 AmplimixLayerImpl::GetPitch() const
    {
        return AMPLIMIX_LOAD_RELAXED(&pitch);
    }

    AmReal32 AmplimixLayerImpl::GetObstruction() const
    {
        return AMPLIMIX_LOAD_RELAXED(&obstruction);
    }

    AmReal32 AmplimixLayerImpl::GetOcclusion() const
    {
        return AMPLIMIX_LOAD_RELAXED(&occlusion);
    }

    AmReal32 AmplimixLayerImpl::GetPlaySpeed() const
    {
        return AMPLIMIX_LOAD_RELAXED(&playSpeed);
    }

    AmVector3 AmplimixLayerImpl::GetLocation() const
    {
        if (snd == nullptr || snd->sound == nullptr)
            return kVector3Zero;

        if (processingInstance && currentInstanceIndex < instanceData.size())
            return instanceData[currentInstanceIndex].location;

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

    Room AmplimixLayerImpl::GetRoom() const
    {
        if (snd == nullptr || snd->sound == nullptr)
            return Room(nullptr);

        if (processingInstance && currentInstanceIndex < instanceData.size())
            return instanceData[currentInstanceIndex].room;

        return snd->sound->GetChannel().GetRoom();
    }

    Channel AmplimixLayerImpl::GetChannel() const
    {
        if (snd == nullptr || snd->sound == nullptr)
            return Channel(nullptr);

        return snd->sound->GetChannel();
    }

    Bus AmplimixLayerImpl::GetBus() const
    {
        if (snd == nullptr || snd->sound == nullptr)
            return Bus(nullptr);

        return amEngine->FindBus(snd->sound->GetSettings().m_busID);
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

    const std::shared_ptr<EffectInstance> AmplimixLayerImpl::GetEffect() const
    {
        if (snd == nullptr || snd->sound == nullptr)
            return nullptr;

        return snd->sound->GetEffect();
    }

    const Attenuation* AmplimixLayerImpl::GetAttenuation() const
    {
        if (snd == nullptr || snd->sound == nullptr)
            return nullptr;

        return amEngine->GetAttenuationHandle(snd->sound->GetSettings().m_attenuationID);
    }

    AmUInt32 AmplimixLayerImpl::GetSampleRate() const
    {
        if (snd == nullptr || snd->sound == nullptr)
            return 0;

        const AmReal32 ratio = AMPLIMIX_LOAD_RELAXED(&sampleRateRatio);
        return snd->format.GetSampleRate() * ratio;
    }

    bool AmplimixLayerImpl::IsMultiPosition() const
    {
        if (snd == nullptr || snd->sound == nullptr)
            return false;

        if (processingInstance)
            return false;

        const auto& channel = snd->sound->GetChannel();
        if (!channel.Valid())
            return false;

        auto* channelState = channel.GetState();
        return channelState->IsInstancingEnabled() && channelState->GetInstanceCount() > 0;
    }

    eChannelInstanceMode AmplimixLayerImpl::GetInstancingMode() const
    {
        if (snd == nullptr || snd->sound == nullptr)
            return eChannelInstanceMode_Blended;

        const auto& channel = snd->sound->GetChannel();
        if (!channel.Valid())
            return eChannelInstanceMode_Blended;

        return channel.GetState()->GetInstancingMode();
    }

    AmSize AmplimixLayerImpl::GetInstanceCount() const
    {
        return instanceData.size();
    }

    AmVector3 AmplimixLayerImpl::GetInstanceLocation(AmSize index) const
    {
        if (index >= instanceData.size())
            return kVector3Zero;

        return instanceData[index].location;
    }

    Room AmplimixLayerImpl::GetInstanceRoom(AmSize index) const
    {
        if (index >= instanceData.size())
            return Room();

        return instanceData[index].room;
    }

    AmReal32 AmplimixLayerImpl::GetInstanceWeight(AmSize index) const
    {
        if (index >= instanceData.size())
            return 1.0f;

        return instanceData[index].weight;
    }

    AmReal32 AmplimixLayerImpl::GetInstanceGain(AmSize index) const
    {
        if (index >= instanceData.size())
            return 1.0f;

        return instanceData[index].computedGain;
    }

    void AmplimixLayerImpl::UpdateInstanceData()
    {
        previousInstanceData.clear();
        previousInstanceData.swap(instanceData);

        if (snd == nullptr || snd->sound == nullptr)
            return;

        const auto& channel = snd->sound->GetChannel();
        if (!channel.Valid())
            return;

        auto* channelState = channel.GetState();
        if (!channelState->IsInstancingEnabled())
            return;

        const auto* snapshot = channelState->AcquireInstanceSnapshot();

        instanceData.reserve(snapshot->size());
        for (const auto& entry : *snapshot)
        {
            InstanceData data = entry;

            // Preserve the layer-side cursor for instances that were already present;
            // the snapshot's cursor is only the initial value for (re)discovered ones.
            for (const auto& prev : previousInstanceData)
            {
                if (prev.instanceId == entry.instanceId)
                {
                    data.cursor = prev.cursor;
                    break;
                }
            }

            instanceData.push_back(data);
        }
    }
} // namespace SparkyStudios::Audio::Amplitude
