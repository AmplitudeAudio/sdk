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

    // Audio thread: whether a mixer command may still act on the layer. A detached layer belongs to the game thread,
    // which destroys it; the audio thread never touches it again.
    static bool IsLiveForCommand(const AmplimixLayerImpl* layer, AmUInt32 id)
    {
        return layer->slot.load(std::memory_order_acquire) == eLayerSlot::Live && !layer->detached.load(std::memory_order_relaxed) &&
            layer->id == id;
    }

    AmplimixImpl::AmplimixImpl(AmReal32 masterGain)
        : _initialized(false)
        , _commandsStack()
        , _audioThreadMutex()
        , _nextId(0)
        , _masterGain()
        , _layers()
        , _voiceCommands(std::make_unique<VoiceCommandQueue>())
        , _voiceEvents(std::make_unique<VoiceEventQueue>())
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

        _maxInstancePipelines = config->mixer()->max_instance_pipelines();

        UpdateVoiceBlockFrames();
        _voiceOutbox.reserve(256);
        _pendingDestroys.reserve(256);

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

        // Drain any pending deferred commands. No mix can run any more: layers are destroyed here synchronously.
        ExecuteCommands();

        for (auto& layer : _layers)
            layer.Destroy();

        _pendingDestroys.clear();

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

        UpdateVoiceBlockFrames();

        // Publish to atomic snapshots for audio-thread reads
        AMPLIMIX_STORE_RELAXED(&_mixOutputSampleRate, _device.mRequestedOutputSampleRate);
        AMPLIMIX_STORE_RELAXED(&_mixOutputChannels, _device.mRequestedOutputChannels);
    }

    void AmplimixImpl::SetAfterMixCallback(AfterMixCallback callback)
    {
        _afterMixCallback = callback;
    }

    void AmplimixImpl::UpdateVoiceBlockFrames()
    {
        const auto channels = static_cast<AmUInt64>(_device.mRequestedOutputChannels);
        _voiceBlockFrames = channels > 0 ? AM_MAX(static_cast<AmUInt64>(_device.mOutputBufferSize) / channels, 1ULL) : 1ULL;
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

        // Mixer commands pushed before a transport command (e.g. an instance stream attached before a resume) apply
        // before it; transport commands flushed by the game thread then land on the first frame of this block.
        ExecuteCommands();
        DrainVoiceCommands();

        const AmUInt64 clock = _audioClock.load(std::memory_order_relaxed);
        const AmUInt64 step = _voiceBlockFrames > 0 ? _voiceBlockFrames : frameCount;
        const AmUInt32 activeLayerCount = AMPLIMIX_LOAD_RELAXED(&_activeLayerCount);
        bool hasMixedAtLeastOneLayer = false;

        // Callbacks larger than the nominal block are rendered in nominal sub-blocks: voices size their buffers once.
        for (AmUInt64 offset = 0; offset < frameCount; offset += step)
        {
            const AmUInt64 frames = AM_MIN(step, frameCount - offset);

            for (AmUInt32 i = 0; i < activeLayerCount; ++i)
            {
                if (_engineState->stopping.load(std::memory_order_relaxed))
                    break; // Stop mixing if engine is stopping

                auto& layer = _layers[_activeLayerIndices[i]];

                if (!ShouldMix(&layer))
                {
                    // A finished voice keeps the events a full queue could not take: publish them once there is room.
                    if (layer.voice != nullptr && layer.slot.load(std::memory_order_acquire) == eLayerSlot::Live)
                        layer.voice->GetMailbox().Publish(*_voiceEvents);

                    continue;
                }

                if (offset == 0)
                    layer.UpdateInstanceData();

                hasMixedAtLeastOneLayer |= MixVoice(&layer, &_scratchBuffer, offset, frames, clock + offset);
            }
        }

        _audioClock.store(clock + frameCount, std::memory_order_release);
        ExecuteCommands();

        if (!hasMixedAtLeastOneLayer)
            return 0;

        // Run the after-mix callback if available
        if (_afterMixCallback != nullptr)
            _afterMixCallback(this, &_scratchBuffer, frameCount);

        if (outBuffer != nullptr)
            *outBuffer = &_scratchBuffer;

        return frameCount;
    }

    AmUInt32 AmplimixImpl::StartVoice(SoundData* sound, const VoiceStartOptions& options, AmUInt32 id, AmUInt32 layer)
    {
        if (sound == nullptr || sound->length == 0)
            return 0;

        // define a layer id
        layer = layer == 0 ? ++_nextId : layer;

        // skip 0 as it is special
        if (id == 0)
            id = kAmplimixLayersCount;

        auto* lay = GetLayer(layer);
        if (lay->slot.load(std::memory_order_acquire) != eLayerSlot::Free)
            return 0;

        const AmUInt32 outputRate = AMPLIMIX_LOAD_RELAXED(&_mixOutputSampleRate);

        VoiceSettings settings;
        settings.source = MakeVoiceSource(sound);
        settings.regionStart = 0;
        settings.regionEnd = sound->length;
        settings.loop = options.loop;
        settings.loopCount = options.loopCount;
        settings.startPosition = options.startPosition;
        settings.startFrame = options.startFrame;
        settings.fadeIn = options.fadeIn;
        settings.outputRate = outputRate;
        settings.maxBlockFrames = _voiceBlockFrames;
        settings.resamplerName = _resamplerName;
        settings.curve = options.faderName.empty() ? nullptr : Fader::Construct(options.faderName);
        settings.speed = AM_MAX(options.pitch * options.speed, 0.001f);
        settings.layer = layer;
        settings.id = id;

        auto* voice = ampoolnew(eMemoryPoolKind_Amplimix, Voice);
        if (!voice->Initialize(settings))
        {
            ampooldelete(eMemoryPoolKind_Amplimix, Voice, voice);
            amLogError("Cannot start the voice: invalid source or output format.");
            return 0;
        }

        if (_pipeline == nullptr)
            amLogWarning(
                "No active pipeline is set, this sound will not be rendered. Configure the Amplimix pipeline in the engine configuration.");

        // The layer is Free and not in the active list: the audio thread does not touch it until ActivateLayer runs.
        lay->pipeline = _pipeline != nullptr ? _pipeline->CreateInstance(lay) : nullptr;
        lay->id = id;
        lay->snd = sound;
        lay->voice = voice;
        lay->start = 0;
        lay->end = sound->length;
        lay->currentSpeed = static_cast<AmReal32>(settings.speed);
        lay->releaseRequested = false;
        lay->detached.store(false, std::memory_order_relaxed);
        lay->voiceState.store(voice->GetPublishedState(), std::memory_order_relaxed);
        lay->voicePosition.store(voice->GetPublishedPosition(), std::memory_order_relaxed);
        AMPLIMIX_STORE(&lay->gain, options.gain);
        AMPLIMIX_STORE(&lay->pitch, options.pitch);
        AMPLIMIX_STORE(&lay->userPlaySpeed, options.speed);

        // Pre-warm the pool so the audio thread never allocates: a mono render and a stereo pipeline output.
        lay->_chunkPool.PreWarm(_voiceBlockFrames, kAmMonoChannelCount, _voiceBlockFrames);
        {
            SoundChunk* mono = lay->_chunkPool.Acquire(_voiceBlockFrames, kAmMonoChannelCount, false);
            lay->pipelineFrames = mono->buffer->GetFrameCount();
            lay->_chunkPool.Release(mono);
        }

        // The first mixed block snaps to the target gain; later changes ramp. These writes are published to the audio
        // thread by the release-store to lay->slot below.
        for (auto& processor : lay->_mixGain)
        {
            processor.Invalidate();
            processor.SetMinRampFrames(GainRampMinFrames(outputRate));
        }

        lay->slot.store(eLayerSlot::Live, std::memory_order_release);

        PushCommand(
            { [this, lay]() -> bool
              {
                  // Add to active layer list
                  ActivateLayer(GetLayerIndex(lay));
                  return true;
              } });

        return layer;
    }

    bool AmplimixImpl::SetObstruction(AmUInt32 id, AmUInt32 layer, AmReal32 obstruction)
    {
        auto* lay = GetLayer(layer);

        if (lay->slot.load(std::memory_order_acquire) != eLayerSlot::Live || id != lay->id)
            return false;

        AMPLIMIX_STORE(&lay->obstruction, obstruction);

        return true;
    }

    bool AmplimixImpl::SetOcclusion(AmUInt32 id, AmUInt32 layer, AmReal32 occlusion)
    {
        auto* lay = GetLayer(layer);

        if (lay->slot.load(std::memory_order_acquire) != eLayerSlot::Live || id != lay->id)
            return false;

        AMPLIMIX_STORE(&lay->occlusion, occlusion);

        return true;
    }

    bool AmplimixImpl::SetGain(AmUInt32 id, AmUInt32 layer, AmReal32 gain)
    {
        auto* lay = GetLayer(layer);

        if (lay->slot.load(std::memory_order_acquire) != eLayerSlot::Live || id != lay->id)
            return false;

        AMPLIMIX_STORE(&lay->gain, gain);

        return true;
    }

    bool AmplimixImpl::SetPitch(AmUInt32 id, AmUInt32 layer, AmReal32 pitch)
    {
        auto* lay = GetLayer(layer);

        if (lay->slot.load(std::memory_order_acquire) != eLayerSlot::Live || id != lay->id)
            return false;

        AMPLIMIX_STORE(&lay->pitch, pitch);

        return true;
    }

    bool AmplimixImpl::SetPlaySpeed(AmUInt32 id, AmUInt32 layer, AmReal32 speed)
    {
        auto* lay = GetLayer(layer);

        if (lay->slot.load(std::memory_order_acquire) != eLayerSlot::Live || id != lay->id)
            return false;

        AMPLIMIX_STORE(&lay->userPlaySpeed, speed);

        return true;
    }

    AmSize AmplimixImpl::GetMaxInstancePipelines() const
    {
        return _maxInstancePipelines;
    }

    void AmplimixImpl::SetMaxInstancePipelines(AmSize count)
    {
        _maxInstancePipelines = count;
    }

    bool AmplimixImpl::InstallInstancePipelineTable(AmUInt32 id, AmUInt32 layer)
    {
        auto* lay = GetLayer(layer);
        if (lay->id != id || lay->slot.load(std::memory_order_acquire) != eLayerSlot::Live)
            return false;

        // Built here, on the game thread, with all their slots: the audio thread never allocates for them.
        auto pipelines = std::make_shared<InstancePipelineTable>(_maxInstancePipelines);
        auto streams = std::make_shared<InstanceStreamTable>(kAmMaxChannelInstances);

        PushCommand(
            { [lay, id, pipelines, streams]() -> bool
              {
                  // Drop the command if the layer finished or was reused since it was pushed.
                  if (!IsLiveForCommand(lay, id))
                      return true;

                  lay->instancePipelines = pipelines;
                  lay->instanceStreams = streams;
                  return true;
              } });

        return true;
    }

    bool AmplimixImpl::AttachInstancePipeline(AmUInt32 id, AmUInt32 layer, AmChannelInstanceID instanceId)
    {
        if (_pipeline == nullptr)
            return false;

        if (!PinLayer(id, layer))
            return false;

        struct LayerPinGuard
        {
            AmplimixImpl* mixer;
            AmUInt32 layer;

            ~LayerPinGuard()
            {
                mixer->UnpinLayer(layer);
            }
        } pinGuard{ .mixer = this, .layer = layer };

        // Pinned: the layer cannot be destroyed or reused while the pipeline is built. pipelineFrames is written by
        // StartVoice() on this thread.
        auto* lay = GetLayer(layer);
        const AmUInt64 frames = lay->pipelineFrames;

        // Created and configured on the game thread, so the audio thread is never blocked by it. Configuring here, with
        // the mono input and stereo output the mixer passes to Execute(), keeps the first Execute() from allocating.
        std::shared_ptr<PipelineInstance> pipeline = _pipeline->CreateInstance(lay);
        if (pipeline == nullptr)
            return false;

        if (frames > 0)
            pipeline->Configure(frames, static_cast<AmUInt16>(kAmMonoChannelCount), frames, static_cast<AmUInt16>(kAmStereoChannelCount));

        if (lay->id != id || lay->slot.load(std::memory_order_acquire) != eLayerSlot::Live)
            return false;

        PushCommand(
            { [lay, id, instanceId, pipeline]() -> bool
              {
                  if (!IsLiveForCommand(lay, id) || lay->instancePipelines == nullptr)
                      return true;

                  AM_UNUSED(lay->instancePipelines->Attach(instanceId, pipeline));
                  return true;
              } });

        return true;
    }

    bool AmplimixImpl::DetachInstancePipeline(AmUInt32 id, AmUInt32 layer, AmChannelInstanceID instanceId)
    {
        AmplimixMutexLocker lock(this);

        auto* lay = GetLayer(layer);
        if (lay->id != id || lay->slot.load(std::memory_order_acquire) != eLayerSlot::Live)
            return false;

        PushCommand(
            { [lay, id, instanceId]() -> bool
              {
                  if (!IsLiveForCommand(lay, id) || lay->instancePipelines == nullptr)
                      return true;

                  // Released at the end of this command, after the mix.
                  AM_UNUSED(lay->instancePipelines->Detach(instanceId));
                  return true;
              } });

        return true;
    }

    AmSize AmplimixImpl::GetInstancePipelineCount(AmUInt32 id, AmUInt32 layer) const
    {
        const AmplimixLayerImpl& lay = _layers[layer & kAmplimixLayersMask];
        if (lay.id != id || lay.slot.load(std::memory_order_acquire) != eLayerSlot::Live || lay.instancePipelines == nullptr)
            return 0;

        return lay.instancePipelines->GetSize();
    }

    bool AmplimixImpl::PinLayer(AmUInt32 id, AmUInt32 layer)
    {
        AmplimixMutexLocker lock(this);

        auto* lay = GetLayer(layer);
        lay->pins.fetch_add(1, std::memory_order_seq_cst);

        // Pairs with the fence in TryDestroyLayer(): either this check sees the destroy, or the destroy sees the pin.
        std::atomic_thread_fence(std::memory_order_seq_cst);

        if (lay->destroyPending.load(std::memory_order_seq_cst) || lay->id != id ||
            lay->slot.load(std::memory_order_acquire) != eLayerSlot::Live)
        {
            UnpinLayer(layer);
            return false;
        }

        return true;
    }

    void AmplimixImpl::UnpinLayer(AmUInt32 layer)
    {
        // A released layer skipped while pinned is destroyed by the next DestroyDetachedLayers() sweep.
        GetLayer(layer)->pins.fetch_sub(1, std::memory_order_seq_cst);
    }

    void AmplimixImpl::SetMasterGain(AmReal32 gain)
    {
        AMPLIMIX_STORE_RELAXED(&_masterGain, gain);
    }

    void AmplimixImpl::StopAll()
    {
        for (AmUInt32 i = 0; i < kAmplimixLayersCount; ++i)
            if (_layers[i].slot.load(std::memory_order_acquire) == eLayerSlot::Live)
                PostVoiceCommand(_layers[i].id, i, eVoiceCommandKind::Stop);
    }

    void AmplimixImpl::HaltAll()
    {
        for (AmUInt32 i = 0; i < kAmplimixLayersCount; ++i)
            if (_layers[i].slot.load(std::memory_order_acquire) == eLayerSlot::Live)
                PostVoiceCommand(_layers[i].id, i, eVoiceCommandKind::Pause);
    }

    void AmplimixImpl::PlayAll()
    {
        for (AmUInt32 i = 0; i < kAmplimixLayersCount; ++i)
            if (_layers[i].slot.load(std::memory_order_acquire) == eLayerSlot::Live)
                PostVoiceCommand(_layers[i].id, i, eVoiceCommandKind::Resume);
    }

    void AmplimixImpl::PostVoiceCommand(
        AmUInt32 id, AmUInt32 layer, eVoiceCommandKind kind, AmTime duration, AmUInt64 position, AmUInt64 frame)
    {
        std::lock_guard<std::mutex> lock(_voiceOutboxMutex);
        _voiceOutbox.push_back(VoiceCommand{ layer, id, kind, frame, duration, position });
    }

    void AmplimixImpl::FlushVoiceCommands()
    {
        std::lock_guard<std::mutex> lock(_voiceOutboxMutex);

        // A full queue keeps the rest for the next frame: nothing is dropped.
        AmSize sent = 0;
        while (sent < _voiceOutbox.size() && _voiceCommands->TryEnqueue(_voiceOutbox[sent]))
            ++sent;

        _voiceOutbox.erase(_voiceOutbox.begin(), _voiceOutbox.begin() + static_cast<std::ptrdiff_t>(sent));
    }

    void AmplimixImpl::DrainVoiceCommands()
    {
        VoiceCommand command;
        while (_voiceCommands->TryDequeue(command))
        {
            auto* lay = GetLayer(command.layer);
            if (!IsLiveForCommand(lay, command.id) || lay->voice == nullptr)
                continue;

            lay->voice->Enqueue(command);
        }
    }

    eVoiceState AmplimixImpl::GetVoiceState(AmUInt32 id, AmUInt32 layer) const
    {
        const AmplimixLayerImpl& lay = _layers[layer & kAmplimixLayersMask];
        // Read from the layer, never from the voice: the audio thread may destroy the voice once its release is pushed.
        if (lay.id != id || lay.slot.load(std::memory_order_acquire) != eLayerSlot::Live)
            return eVoiceState::Idle;

        return lay.voiceState.load(std::memory_order_acquire);
    }

    bool AmplimixImpl::GetVoicePosition(AmUInt32 id, AmUInt32 layer, AmUInt64& position) const
    {
        const AmplimixLayerImpl& lay = _layers[layer & kAmplimixLayersMask];
        if (lay.id != id || lay.slot.load(std::memory_order_acquire) != eLayerSlot::Live)
            return false;

        position = lay.voicePosition.load(std::memory_order_acquire);
        return true;
    }

    AmUInt64 AmplimixImpl::GetAudioClock() const
    {
        return _audioClock.load(std::memory_order_acquire);
    }

    void AmplimixImpl::ReleaseLayer(AmUInt32 id, AmUInt32 layer)
    {
        auto* lay = GetLayer(layer);
        if (lay->id != id || lay->slot.load(std::memory_order_acquire) != eLayerSlot::Live || lay->releaseRequested)
            return;

        // Game thread only: from here on nothing on the game thread reads the voice or the sound of this layer.
        lay->releaseRequested = true;

        // The audio thread only takes the layer out of its active list; the game thread destroys it once it sees the
        // layer detached and unpinned (DestroyDetachedLayers), so the audio thread never frees or locks for it.
        PushCommand(
            { [this, lay, id]() -> bool
              {
                  if (!IsLiveForCommand(lay, id))
                      return true;

                  DeactivateLayer(GetLayerIndex(lay));
                  lay->detached.store(true, std::memory_order_release);
                  return true;
              } });

        _pendingDestroys.push_back(GetLayerIndex(lay));
    }

    bool AmplimixImpl::TryDestroyLayer(AmplimixLayerImpl* layer)
    {
        if (!layer->detached.load(std::memory_order_acquire))
            return false;

        // Pairs with the fence in PinLayer(): either this check sees the pin, or the pin sees the destroy and fails.
        layer->destroyPending.store(true, std::memory_order_seq_cst);
        std::atomic_thread_fence(std::memory_order_seq_cst);

        if (layer->pins.load(std::memory_order_seq_cst) > 0)
        {
            layer->destroyPending.store(false, std::memory_order_seq_cst);
            return false;
        }

        layer->Destroy();
        return true;
    }

    void AmplimixImpl::DestroyDetachedLayers()
    {
        if (_pendingDestroys.empty())
            return;

        // Destroying a layer deletes its sound instance, which may release a sound and halt channels: work on a copy so
        // a release made meanwhile lands in the list for the next sweep.
        std::vector<AmUInt32> pending;
        pending.swap(_pendingDestroys);

        for (const AmUInt32 index : pending)
            if (!TryDestroyLayer(&_layers[index]))
                _pendingDestroys.push_back(index);
    }

    void AmplimixImpl::DiscardVoice(AmUInt32 id, AmUInt32 layer)
    {
        // The layer owns the sound instance: destroying the layer after the mix deletes it, whether or not the voice
        // finished. The caller must not delete the instance itself.
        ReleaseLayer(id, layer);
    }

    bool AmplimixImpl::AttachInstanceStream(AmUInt32 id, AmUInt32 layer, AmChannelInstanceID instanceId)
    {
        if (!PinLayer(id, layer))
            return false;

        auto* lay = GetLayer(layer);
        auto stream = ampoolshared(eMemoryPoolKind_Amplimix, VoiceStreamSlot);
        const bool ready = lay->voice != nullptr && lay->voice->InitializeSlot(*stream, lay->start);
        UnpinLayer(layer);

        if (!ready)
            return false;

        PushCommand(
            { [lay, id, instanceId, stream]() -> bool
              {
                  if (!IsLiveForCommand(lay, id) || lay->instanceStreams == nullptr)
                      return true;

                  // Start where the instance is: its cursor lives on the audio thread.
                  for (const auto& data : lay->instanceData)
                  {
                      if (data.instanceId != instanceId)
                          continue;

                      stream->reader.Seek(data.cursor);
                      break;
                  }

                  AM_UNUSED(lay->instanceStreams->Attach(instanceId, stream));
                  return true;
              } });

        return true;
    }

    bool AmplimixImpl::DetachInstanceStream(AmUInt32 id, AmUInt32 layer, AmChannelInstanceID instanceId)
    {
        auto* lay = GetLayer(layer);
        if (lay->id != id || lay->slot.load(std::memory_order_acquire) != eLayerSlot::Live)
            return false;

        PushCommand(
            { [lay, id, instanceId]() -> bool
              {
                  if (IsLiveForCommand(lay, id) && lay->instanceStreams != nullptr)
                      AM_UNUSED(lay->instanceStreams->Detach(instanceId)); // released after the mix, like pipelines
                  return true;
              } });

        return true;
    }

    void AmplimixImpl::DispatchVoiceEvents()
    {
        // Layers the audio thread detached since the last frame are destroyed here, on the game thread.
        DestroyDetachedLayers();

        VoiceEvent event;
        while (_voiceEvents->TryDequeue(event))
            HandleVoiceEvent(event);
    }

    void AmplimixImpl::HandleVoiceEvent(const VoiceEvent& event)
    {
        if (event.kind == eVoiceEventKind::Error)
            amLogError("Voice on layer %u failed to render and was stopped.", event.layer);

        AmplimixLayerImpl* layer = GetLayer(event.layer);

        // A layer whose release was pushed may be destroyed by the audio thread at any time: never read its sound.
        if (layer->id != event.id || layer->slot.load(std::memory_order_acquire) != eLayerSlot::Live || layer->releaseRequested ||
            layer->snd == nullptr)
            return;

        SoundInstance* sound = layer->snd->sound.get();
        ChannelInternalState* channelState = sound != nullptr ? sound->GetChannel().GetState() : nullptr;

        // The channel may have been reset and reused since: only the channel that still owns this layer hears the event.
        const bool owned = channelState != nullptr && channelState->GetRealChannel().OwnsMixerLayer(event.layer, sound);

        switch (event.kind)
        {
        case eVoiceEventKind::Started:
            if (owned)
                channelState->Trigger(eChannelEvent_Begin);
            break;

        case eVoiceEventKind::Looped:
            for (AmUInt32 i = 0; i < event.count; ++i)
            {
                IncrementSoundLoopCount(sound);
                if (owned)
                    channelState->Trigger(eChannelEvent_Loop);
            }
            break;

        case eVoiceEventKind::FadedOut:
            if (owned)
                channelState->OnVoiceFadedOut(event.layer, event.target, event.frame, event.sourcePosition);
            break;

        case eVoiceEventKind::Ended:
            if (owned)
                HandleVoiceEnded(layer, channelState);
            break;

        case eVoiceEventKind::Finished:
            if (owned)
                channelState->GetRealChannel().ForgetMixerLayer(event.layer);
            ReleaseLayer(event.id, event.layer);
            break;

        case eVoiceEventKind::Error:
            if (owned)
                channelState->HaltInternal();
            break;

        case eVoiceEventKind::Count:
            break;
        }
    }

    void AmplimixImpl::HandleVoiceEnded(AmplimixLayerImpl* layer, ChannelInternalState* channelState)
    {
        const SoundInstance* sound = layer->snd->sound.get();
        amLogDebug("Ended sound: '%s'.", sound->GetSound()->GetName().c_str());

        const bool stopping = _engineState != nullptr && _engineState->stopping.load(std::memory_order_acquire);
        if (stopping || sound->GetSettings().m_kind != SoundKind::Contained)
        {
            channelState->Trigger(eChannelEvent_End);
            channelState->HaltInternal();
            return;
        }

        // PlayAll collections chain the next sound; PlayOne ones do nothing here.
        const CollectionImpl* collection = sound->GetCollection();
        AMPLITUDE_ASSERT(collection != nullptr); // Should always have a collection for contained sound instances.
        if (collection->GetDefinition()->play_mode() != CollectionPlayMode_PlayAll)
            return;

        RealChannel& realChannel = channelState->GetRealChannel();
        realChannel.MarkAsPlayed(sound->GetSound());
        if (realChannel.AllSoundsHasPlayed())
        {
            realChannel.ClearPlayedSounds();
            channelState->Trigger(eChannelEvent_End);
            channelState->HaltInternal();
        }

        // Play the collection again only if the channel is still playing.
        if (realChannel.Playing())
            channelState->Play();
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

    bool AmplimixImpl::MixVoice(AmplimixLayerImpl* layer, AudioBuffer* buffer, AmUInt64 offset, AmUInt64 frames, AmUInt64 blockClock)
    {
        Voice* voice = layer->voice;
        voice->SetSpeed(UpdateSpeed(layer));
        voice->BeginBlock(blockClock, frames);

        // Amplimix::Init only requests Mono or Stereo output.
        const auto outputChannels = static_cast<AmUInt16>(_device.mRequestedOutputChannels);
        AMPLITUDE_ASSERT(outputChannels >= 1 && outputChannels <= kAmplimixMaxOutputChannels);

        const AmReal32 gain = AMPLIMIX_LOAD(&_masterGain) * AMPLIMIX_LOAD(&layer->gain);
        const bool audible = voice->IsAudible() && layer->pipeline != nullptr;

        if (!audible)
        {
            AdvanceLayerGain(layer->_mixGain, outputChannels, gain, frames);
        }
        else if (IsSeparateMode(layer))
        {
            MixVoiceInstances(layer, buffer, offset, frames, gain, outputChannels);
        }
        else
        {
            SoundChunk* mono = layer->_chunkPool.Acquire(frames, kAmMonoChannelCount);
            SoundChunk* out = layer->_chunkPool.Acquire(mono->frames, kAmStereoChannelCount);

            voice->RenderPrimary(*mono->buffer);
            layer->pipeline->Execute(*mono->buffer, *out->buffer);
            voice->ApplyGain(*out->buffer);
            MixLayerWithGain(layer->_mixGain, outputChannels, gain, *out->buffer, *buffer, offset, frames);

            layer->_chunkPool.Release(out);
            layer->_chunkPool.Release(mono);
        }

        voice->EndBlock();
        layer->voicePosition.store(voice->GetPublishedPosition(), std::memory_order_release);
        layer->voiceState.store(voice->GetPublishedState(), std::memory_order_release);
        voice->GetMailbox().Publish(*_voiceEvents);
        layer->ResetPipeline();

        return audible;
    }

    bool AmplimixImpl::IsSeparateMode(const AmplimixLayerImpl* layer)
    {
        const auto& channel = layer->GetChannel();
        return channel.Valid() && channel.GetState()->IsInstancingEnabled() &&
            channel.GetState()->GetInstancingMode() == eChannelInstanceMode_Separate && !layer->instanceData.empty();
    }

    AmReal64 AmplimixImpl::UpdateSpeed(AmplimixLayerImpl* layer)
    {
        const AmReal32 target = AM_MAX(AMPLIMIX_LOAD(&layer->pitch) * AMPLIMIX_LOAD(&layer->userPlaySpeed), 0.001f);

        if (layer->currentSpeed != target)
        {
            // Ease toward the target, and settle exactly so the resampler is not re-tuned every block.
            const AmReal32 next = Lerp(0.75f, layer->currentSpeed, target);
            layer->currentSpeed = std::abs(next - target) < 1e-6f ? target : next;
        }

        return layer->currentSpeed;
    }

    void AmplimixImpl::MixVoiceInstances(
        AmplimixLayerImpl* layer, AudioBuffer* buffer, AmUInt64 offset, AmUInt64 frames, AmReal32 gain, AmUInt16 outputChannels)
    {
        Voice* voice = layer->voice;

        // Cursor write-back goes through the channel's atomic slots: the game-owned instance containers are never
        // touched from the audio thread.
        auto* channelState = layer->GetChannel().GetState();

        SoundChunk* mono = layer->_chunkPool.Acquire(frames, kAmMonoChannelCount);
        SoundChunk* out = layer->_chunkPool.Acquire(mono->frames, kAmStereoChannelCount);

        bool allFinished = true;
        AmUInt64 lastFinish = 0;

        for (AmSize index = 0; index < layer->instanceData.size(); ++index)
        {
            auto& data = layer->instanceData[index];

            // An instance renders once its stream is attached; until then (one block at most) it is silent.
            VoiceStreamSlot* slot = layer->instanceStreams != nullptr ? layer->instanceStreams->Find(data.instanceId) : nullptr;
            if (slot == nullptr)
            {
                allFinished = false;
                continue;
            }

            if (slot->stream.IsFinished())
                continue;

            // Set instance context for pipeline nodes
            layer->processingInstance = true;
            layer->currentInstanceIndex = index;

            // Instances with their own pipeline keep independent node state; others share the layer pipeline.
            PipelineInstance* instancePipeline =
                layer->instancePipelines != nullptr ? layer->instancePipelines->Find(data.instanceId) : nullptr;
            layer->sharedInstancePipeline = instancePipeline == nullptr;

            mono->buffer->Clear();
            out->buffer->Clear();

            slot->stream.SetSpeed(voice->GetSpeed());
            const auto report = voice->Render(*slot, *mono->buffer);
            (instancePipeline != nullptr ? instancePipeline : layer->pipeline.get())->Execute(*mono->buffer, *out->buffer);
            voice->ApplyGain(*out->buffer);
            MixLayerInstanceWithGain(layer->_mixGain, outputChannels, gain, *out->buffer, *buffer, offset, frames);

            data.cursor = slot->stream.IsFinished() ? layer->end : slot->stream.GetSourcePosition(slot->reader);
            if (report.finished)
                lastFinish = AM_MAX(lastFinish, report.finishedFrame);
            else
                allFinished = false;

            // Write the cursor back through the channel's write-back slots, so the game thread can restore it if this
            // layer is recreated. The id check drops stores that race a re-publication.
            if (auto* slots = channelState->GetInstanceCursorSlots(); slots != nullptr && index < kAmMaxChannelInstances)
            {
                auto& cursorSlot = slots[index];
                if (cursorSlot.id.load(std::memory_order_acquire) == data.instanceId)
                    cursorSlot.cursor.store(data.cursor, std::memory_order_release);
            }

            // Clear per-block caches before the next instance. A per-instance pipeline keeps its DSP state; the shared
            // pipeline is reset between instances.
            if (instancePipeline != nullptr)
            {
                instancePipeline->Reset();
                layer->ResetRoomUpdateFlag();
            }
            else
            {
                layer->ResetPipeline();
            }
        }

        // Every instance mixed through a copy of the layer ramp; advance it once for this block.
        AdvanceLayerGain(layer->_mixGain, outputChannels, gain, frames);

        // Clear instance processing context
        layer->processingInstance = false;
        layer->sharedInstancePipeline = false;
        layer->currentInstanceIndex = 0;

        layer->_chunkPool.Release(out);
        layer->_chunkPool.Release(mono);

        if (allFinished)
            voice->NotifySourcesFinished(lastFinish);
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
        // Acquire pairs with the release in StartVoice(): voice, snd and pipeline are visible once the slot is Live.
        if (layer->slot.load(std::memory_order_acquire) != eLayerSlot::Live || layer->voice == nullptr)
            return false;

        // A finished voice waits for the game thread to release it; nothing left to render.
        return layer->voice->GetState() != eVoiceState::Finished;
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
        // Runs on the game thread once the audio thread detached the layer (AmplimixImpl::TryDestroyLayer), or from
        // Deinit() when no mix can run: the audio thread never reads the layer while it is torn down.
        if (voice != nullptr)
        {
            ampooldelete(eMemoryPoolKind_Amplimix, Voice, voice);
            voice = nullptr;
        }

        pipeline = nullptr;
        instancePipelines = nullptr;
        instanceStreams = nullptr;

        if (snd != nullptr)
        {
            snd->sound.reset();
            snd = nullptr;
        }

        _chunkPool.Reset();
        destroyPending.store(false, std::memory_order_relaxed);
        detached.store(false, std::memory_order_relaxed);
        voiceState.store(eVoiceState::Idle, std::memory_order_relaxed);

        slot.store(eLayerSlot::Free, std::memory_order_release);
    }

    void AmplimixLayerImpl::ResetPipeline()
    {
        // Check if pipeline is valid before resetting
        // The pipeline can be nullptr if the sound is being destroyed on another thread
        if (pipeline != nullptr)
            pipeline->Reset();

        ResetRoomUpdateFlag();
    }

    void AmplimixLayerImpl::ResetRoomUpdateFlag()
    {
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
        return voice != nullptr ? voice->GetPublishedPosition() : start;
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
        return currentSpeed;
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

        return snd->sound->GetSettings().m_attenuation;
    }

    AmUInt32 AmplimixLayerImpl::GetSampleRate() const
    {
        if (snd == nullptr || snd->sound == nullptr || voice == nullptr)
            return 0;

        // Source rate times the playback ratio (source rate / output rate x speed).
        const AmReal64 rate = snd->format.GetSampleRate();
        return static_cast<AmUInt32>(rate * (rate / voice->GetOutputRate()) * currentSpeed);
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

    bool AmplimixLayerImpl::IsSharingPipelineAcrossInstances() const
    {
        return processingInstance && sharedInstancePipeline;
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
