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

#include <algorithm>
#include <optional>
#include <thread>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Core/Engine.h>
#include <Core/Playback/ChannelInternalState.h>
#include <Mixer/Amplimix.h>
#include <Mixer/InstanceBlending.h>
#include <Mixer/InstanceCursors.h>
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
    /// Frames of the stand-in ring a streamed voice is created over, until its planned stream is bound.
    static constexpr AmUInt64 kPlaceholderRingFrames = 16;

    /// Wraps a looping sound of @p loopCount total plays may make.
    static AmUInt32 StreamWraps(bool loop, AmUInt32 loopCount)
    {
        return !loop ? 0 : loopCount == 0 ? SourceReader::kInfiniteWraps : loopCount - 1;
    }

    /// Forgets a stream the layer holds and retires it. Game thread.
    static void RetireHeldStream(AmplimixLayerImpl* layer, StreamSource* stream)
    {
        const auto it = std::find(layer->heldStreams.begin(), layer->heldStreams.end(), stream);
        if (it == layer->heldStreams.end())
            return; // retired already: never twice

        layer->heldStreams.erase(it);
        layer->streaming->Retire(stream);
    }

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
        // Default is resolved against the device when the driver opens it (UpdateDevice).
        _device.mRequestedOutputChannels = static_cast<PlaybackOutputChannels>(config->output()->channels());
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

        // The layout is chosen once: pipelines are configured for it, and the device converts if it changes later.
        if (_device.mRequestedOutputChannels == PlaybackOutputChannels::Default)
            _device.mRequestedOutputChannels = static_cast<PlaybackOutputChannels>(
                ChannelLayout::LargestFitting(static_cast<AmUInt16>(deviceOutputChannels)).GetChannelCount());

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
        // Anything logged while mixing is queued for the game thread: no lock, no allocation, no I/O here.
        const Logger::ScopedAudioThread audioThread;

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
                self->_isMixing.store(false, std::memory_order_release);
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

        // The stream bookkeeping of the layer is published below; keep other game-side threads out meanwhile.
        std::lock_guard streamLock(_streamMutex);

        // define a layer id
        layer = layer == 0 ? ++_nextId : layer;

        // skip 0 as it is special
        if (id == 0)
            id = kAmplimixLayersCount;

        auto* lay = GetLayer(layer);
        if (lay->slot.load(std::memory_order_acquire) != eLayerSlot::Free)
            return 0;

        const AmUInt32 outputRate = AMPLIMIX_LOAD_RELAXED(&_mixOutputSampleRate);

        StreamingThread* streaming = nullptr;
        if (sound->stream)
        {
            streaming = _engineState != nullptr ? _engineState->streaming.get() : nullptr;
            if (streaming == nullptr)
            {
                amLogError("Cannot start the streamed sound on layer %u: the streaming thread is not available.", layer);
                return 0;
            }
        }

        VoiceSettings settings;
        settings.source = MakeVoiceSource(sound);

        // The voice needs a stream to be created over, but the stream plan needs the voice's preroll: the voice starts
        // over this empty, unregistered stand-in until the planned stream is bound below, before the voice is activated.
        std::optional<StreamSource> placeholder;
        if (streaming != nullptr)
        {
            AmUInt64 unused = 0;
            const AmUInt16 channels = sound->format.GetNumChannels();
            placeholder.emplace(
                nullptr, PlanStream(0, sound->length, false, 0, 0, 0, unused), kPlaceholderRingFrames, channels, 1,
                sound->format.GetSampleRate(), true);
            settings.source.stream = &*placeholder;
        }
        settings.regionStart = 0;
        settings.regionEnd = sound->length;
        settings.loop = options.loop;
        settings.loopCount = options.loopCount;
        settings.startPosition = options.startPosition;
        settings.startFrame = options.startFrame;
        settings.startPositionClock = options.startPositionClock;
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

            if (const AmUInt16 channels = settings.source.channels; ChannelLayout::FromChannelCount(channels) == nullptr)
            {
                amLogError(
                    "Cannot start the voice: a %u-channel asset has no channel layout (supported: 1, 2, 4, 6 or 8 channels).",
                    static_cast<AmUInt32>(channels));
            }
            else
            {
                amLogError("Cannot start the voice: invalid source or output format.");
            }
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

        // Pre-warm the pool so the audio thread never allocates: the voice's render and the pipeline's output.
        const AmUInt16 sourceChannels = voice->GetChannelCount();
        const AmUInt16 outputChannels = GetOutputChannelCount();
        lay->_chunkPool.PreWarm(_voiceBlockFrames, sourceChannels, outputChannels);
        {
            SoundChunk* in = lay->_chunkPool.Acquire(_voiceBlockFrames, sourceChannels, false);
            lay->pipelineFrames = in->buffer->GetFrameCount();
            lay->_chunkPool.Release(in);
        }

        // Configured here, on the game thread, with the sound's channels in and the output layout out, so the first
        // Execute() has nothing left to allocate. A graph that cannot render this sound refuses it.
        if (lay->pipeline != nullptr && !lay->pipeline->Configure(lay->pipelineFrames, sourceChannels, lay->pipelineFrames, outputChannels))
        {
            amLogError(
                "Cannot start the voice: the pipeline cannot render a %u-channel sound to %u channels.",
                static_cast<AmUInt32>(sourceChannels), static_cast<AmUInt32>(outputChannels));

            lay->pipeline = nullptr;
            lay->snd = nullptr;
            lay->voice = nullptr;
            ampooldelete(eMemoryPoolKind_Amplimix, Voice, voice);
            return 0;
        }

        // Streamed sounds: take the decoder and plan the first stream. Nothing below this point can fail.
        StreamSource* stream = nullptr;
        if (streaming != nullptr)
        {
            auto decoder = sound->sound->TakeStreamDecoder();
            if (decoder == nullptr)
            {
                amLogError("Cannot start the streamed sound on layer %u: it has no decoder.", layer);

                lay->pipeline = nullptr;
                lay->snd = nullptr;
                lay->voice = nullptr;
                ampooldelete(eMemoryPoolKind_Amplimix, Voice, voice);
                return 0;
            }

            lay->streamLoop = options.loop;
            lay->streamWraps = StreamWraps(options.loop, options.loopCount);
            lay->prerollFrames = voice->GetPrerollFrames();

            AmUInt64 preroll = 0;
            const StreamPlan plan = PlanStream(
                0, sound->length, options.loop, lay->streamWraps, std::min(options.startPosition, sound->length), lay->prerollFrames,
                preroll);

            stream = streaming->Create(std::move(decoder), plan, sound->format.GetNumChannels(), sound->format.GetSampleRate());
            streaming->WakeUp();
            voice->BindStream(stream);

            lay->streaming = streaming;
            lay->heldStreams.push_back(stream);
        }

        lay->stream = stream;
        lay->pendingStream = nullptr;
        lay->pendingPosition = 0;
        lay->pendingClock = kVoiceAsap;
        lay->swapPosted = false;
        lay->hasQueuedSeek = false;
        lay->activationPending = false;
        lay->suppressBegin = false;
        lay->underrunWarned = false;
        lay->dropsLost = false;
        lay->loopsSeen = 0;

        // The first mixed block snaps to the target gain; later changes ramp. These writes are published to the audio
        // thread by the release-store to lay->slot below.
        for (auto& processor : lay->_mixGain)
        {
            processor.Invalidate();
            processor.SetMinRampFrames(GainRampMinFrames(outputRate));
        }

        lay->slot.store(eLayerSlot::Live, std::memory_order_release);

        // A streamed voice is activated once its first stream holds the prefill (UpdateStreams), so it never starts on silence.
        if (stream == nullptr || streaming->IsReady(stream))
            PushActivation(lay);
        else
            lay->activationPending = true;

        return layer;
    }

    const ChannelLayout& AmplimixImpl::GetOutputLayout() const
    {
        return ChannelLayout::LargestFitting(GetOutputChannelCount());
    }

    AmUInt16 AmplimixImpl::GetOutputChannelCount() const
    {
        // No driver has called UpdateDevice yet: the output is stereo until the device reports its own layout.
        if (_device.mRequestedOutputChannels == PlaybackOutputChannels::Default)
            return 2;

        return static_cast<AmUInt16>(_device.mRequestedOutputChannels);
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
        // the sound's channels in and the output layout out, keeps the first Execute() from allocating.
        std::shared_ptr<PipelineInstance> pipeline = _pipeline->CreateInstance(lay);
        if (pipeline == nullptr)
            return false;

        if (frames > 0 && !pipeline->Configure(frames, lay->voice->GetChannelCount(), frames, GetOutputChannelCount()))
        {
            amLogError(
                "Cannot attach the instance pipeline: it cannot render a %u-channel sound to %u channels.",
                static_cast<AmUInt32>(lay->voice->GetChannelCount()), static_cast<AmUInt32>(GetOutputChannelCount()));
            return false;
        }

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
        AmUInt32 id,
        AmUInt32 layer,
        eVoiceCommandKind kind,
        AmTime duration,
        AmUInt64 position,
        AmUInt64 frame,
        StreamSource* stream)
    {
        // A stop that reaches a streamed layer still waiting for its prefill ends it before it was ever heard: the voice
        // starts and stops in the same block, and the channel must not announce a beginning.
        if (kind == eVoiceCommandKind::Stop || kind == eVoiceCommandKind::Release)
        {
            std::lock_guard streamLock(_streamMutex);

            if (auto* lay = GetLayer(layer); lay->id == id && lay->activationPending)
                lay->suppressBegin = true;
        }

        std::lock_guard<std::mutex> lock(_voiceOutboxMutex);
        _voiceOutbox.push_back(VoiceCommand{ layer, id, kind, frame, duration, position, stream });
    }

    void AmplimixImpl::PushActivation(AmplimixLayerImpl* layer)
    {
        PushCommand(
            { [this, layer]() -> bool
              {
                  // Add to active layer list
                  ActivateLayer(GetLayerIndex(layer));
                  return true;
              } });
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

        {
            std::lock_guard streamLock(_streamMutex);
            layer->Destroy();
        }

        return true;
    }

    void AmplimixImpl::DestroyDetachedLayers()
    {
        if (_pendingDestroys.empty())
            return;

        // Destroying a layer deletes its sound instance, which may release a sound and halt channels: work on a copy so
        // a release made meanwhile lands in the list for the next sweep.
        std::vector<AmUInt32> pending;
        pending.reserve(_pendingDestroys.capacity());
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

                  // Discard the filter's group delay, so the instance lines up with the primary path.
                  lay->voice->PrimeStream(*stream);

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

    bool AmplimixImpl::SeekVoice(AmUInt32 id, AmUInt32 layer, AmUInt64 position, AmUInt64 clock)
    {
        std::lock_guard streamLock(_streamMutex);

        auto* lay = GetLayer(layer);

        // In-memory voices seek in place.
        if (lay->streaming == nullptr || lay->id != id)
        {
            PostVoiceCommand(id, layer, eVoiceCommandKind::Seek, 0.0, position, clock);
            return true;
        }

        if (lay->slot.load(std::memory_order_acquire) != eLayerSlot::Live || lay->releaseRequested || lay->snd == nullptr)
            return false;

        return SeekStreamedLayer(lay, position, clock);
    }

    bool AmplimixImpl::SeekStreamedLayer(AmplimixLayerImpl* layer, AmUInt64 position, AmUInt64 clock)
    {
        // A swap is in flight: the newest seek waits for it. It never created a stream, so a superseded one costs nothing.
        if (layer->swapPosted)
        {
            layer->queuedPosition = position;
            layer->queuedClock = clock;
            layer->hasQueuedSeek = true;
            return true;
        }

        StreamingThread* streaming = layer->streaming;

        // A pending stream the audio thread never saw is superseded: nothing reads it, free it now.
        if (layer->pendingStream != nullptr)
        {
            RetireHeldStream(layer, layer->pendingStream);
            layer->pendingStream = nullptr;
        }

        // Every streamed seek gets its own decoder position: the ring of the stream being read must never be rewound.
        auto decoder = layer->snd->sound->OpenStreamDecoder();
        if (decoder == nullptr)
            return false;

        // Wraps left: what the sound started with minus the seams the voice reported. The reports lag the voice, so this
        // may over-estimate; a longer plan only decodes more than the voice ends up reading.
        AmUInt32 wraps = layer->streamWraps;
        if (layer->streamLoop && wraps != SourceReader::kInfiniteWraps)
            wraps = wraps > layer->loopsSeen ? wraps - layer->loopsSeen : 0;

        AmUInt64 preroll = 0;
        const StreamPlan plan =
            PlanStream(layer->start, layer->end, layer->streamLoop, wraps, std::min(position, layer->end), layer->prerollFrames, preroll);

        const SoundFormat& format = layer->snd->format;
        StreamSource* stream = streaming->Create(std::move(decoder), plan, format.GetNumChannels(), format.GetSampleRate());
        streaming->WakeUp();

        layer->heldStreams.push_back(stream);
        layer->pendingStream = stream;
        layer->pendingPosition = position;
        layer->pendingClock = clock;

        // Synchronous (offline) streams are ready at once: the seek lands exactly when it was requested.
        if (streaming->IsReady(stream))
            PostStreamSeek(layer);

        return true;
    }

    void AmplimixImpl::PostStreamSeek(AmplimixLayerImpl* layer)
    {
        if (layer->pendingStream == nullptr || layer->swapPosted)
            return;

        PostVoiceCommand(
            layer->id, GetLayerIndex(layer), eVoiceCommandKind::Seek, 0.0, layer->pendingPosition, layer->pendingClock,
            layer->pendingStream);
        layer->swapPosted = true;
    }

    void AmplimixImpl::UpdateStreams()
    {
        StreamingThread* streaming = _engineState != nullptr ? _engineState->streaming.get() : nullptr;
        if (streaming == nullptr)
            return;

        std::lock_guard streamLock(_streamMutex);

        streaming->CollectRetired();

        for (auto& lay : _layers)
        {
            if (lay.streaming == nullptr || lay.slot.load(std::memory_order_acquire) != eLayerSlot::Live || lay.releaseRequested)
                continue;

            if (lay.activationPending && streaming->IsReady(lay.stream))
            {
                lay.activationPending = false;
                PushActivation(&lay);
            }

            if (lay.pendingStream != nullptr && !lay.swapPosted && streaming->IsReady(lay.pendingStream))
                PostStreamSeek(&lay);
        }
    }

    void AmplimixImpl::HandleStreamSwapped(AmplimixLayerImpl* layer, const StreamSource* stream)
    {
        // Informational: the voice reads @p stream now. Nothing is retired here: the previous stream is reported by
        // StreamDropped.
        if (stream == nullptr || stream == layer->stream || stream != layer->pendingStream)
            return;

        layer->stream = layer->pendingStream;
        layer->pendingStream = nullptr;
        layer->swapPosted = false;

        RunQueuedSeek(layer);
    }

    void AmplimixImpl::HandleStreamDropped(AmplimixLayerImpl* layer, StreamSource* stream)
    {
        if (stream == nullptr)
        {
            // A drop was lost: retire nothing because of it, the layer keeps its streams until it is destroyed.
            layer->dropsLost = true;
            return;
        }

        if (std::find(layer->heldStreams.begin(), layer->heldStreams.end(), stream) == layer->heldStreams.end())
            return; // not ours any more: retired already

        if (stream == layer->stream)
        {
            // The voice cannot drop its current stream in a correct run: keep it (the layer retires it at destroy).
            amLogWarning("The voice of layer %u dropped the stream it was reading.", GetLayerIndex(layer));
            return;
        }

        const bool wasPending = stream == layer->pendingStream;
        RetireHeldStream(layer, stream);

        if (wasPending)
        {
            // The in-flight seek ended without a swap: the voice is free for the next one.
            layer->pendingStream = nullptr;
            layer->swapPosted = false;
            RunQueuedSeek(layer);
        }
    }

    void AmplimixImpl::RunQueuedSeek(AmplimixLayerImpl* layer)
    {
        if (!layer->hasQueuedSeek || layer->releaseRequested)
            return;

        layer->hasQueuedSeek = false;
        SeekStreamedLayer(layer, layer->queuedPosition, layer->queuedClock);
    }

    void AmplimixImpl::DispatchVoiceEvents()
    {
        // Streams the voices dropped are freed, and the ones that are ready start or swap.
        UpdateStreams();

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

        // Stream events work on bookkeeping the layer keeps until it is destroyed, so they apply to a released layer too:
        // a dropped stream is retired at once, and the rest is retired with the layer.
        if (layer->id == event.id && layer->slot.load(std::memory_order_acquire) == eLayerSlot::Live &&
            layer->streaming != nullptr)
        {
            switch (event.kind)
            {
            case eVoiceEventKind::StreamDropped:
                {
                    std::lock_guard streamLock(_streamMutex);
                    HandleStreamDropped(layer, const_cast<StreamSource*>(event.stream));
                    return;
                }

            case eVoiceEventKind::StreamSwapped:
                {
                    std::lock_guard streamLock(_streamMutex);
                    HandleStreamSwapped(layer, event.stream);
                    return;
                }

            case eVoiceEventKind::Underrun:
                layer->streaming->RecordUnderrun();

                if (!layer->underrunWarned && !layer->releaseRequested && layer->snd != nullptr && layer->snd->sound != nullptr)
                {
                    layer->underrunWarned = true;
                    amLogWarning(
                        "Stream underrun on '%s': the streaming thread could not keep up. Consider a larger streaming buffer.",
                        layer->snd->sound->GetSound()->GetName().c_str());
                }
                return;

            default:
                break;
            }
        }

        // A layer whose release was requested is destroyed on the game thread at the next sweep: never read its sound.
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
            {
                const AmUInt64 requested = channelState->GetRealChannel().GetRequestedStartFrame(event.layer);
                if (requested != kVoiceAsap && event.frame > requested)
                    amLogDebug("Scheduled start landed %llu frames late.", static_cast<unsigned long long>(event.frame - requested));

                // Game thread only field: set when a stop reached the layer before it was activated.
                if (!layer->suppressBegin)
                    channelState->Trigger(eChannelEvent_Begin);
            }
            break;

        case eVoiceEventKind::Looped:
            layer->loopsSeen += event.count;
            for (AmUInt32 i = 0; i < event.count; ++i)
            {
                IncrementSoundLoopCount(sound);
                if (owned)
                    channelState->Trigger(eChannelEvent_Loop);
            }
            break;

        case eVoiceEventKind::Underrun:
        case eVoiceEventKind::StreamSwapped:
        case eVoiceEventKind::StreamDropped:
            // Handled above, for streamed layers only.
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

        // Play the collection again only if the channel is still playing: not while it fades toward a stop.
        if (!channelState->IsFadingOutToStopped() && realChannel.Playing())
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
        UpdateSpeed(layer, frames);
        voice->BeginBlock(blockClock, frames);

        const AmUInt16 outputChannels = GetOutputChannelCount();
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
            SoundChunk* source = layer->_chunkPool.Acquire(frames, voice->GetChannelCount());
            SoundChunk* out = layer->_chunkPool.Acquire(source->frames, outputChannels);

            voice->RenderPrimary(*source->buffer);
            layer->pipeline->Execute(*source->buffer, *out->buffer);
            voice->ApplyGain(*out->buffer);
            MixLayerWithGain(layer->_mixGain, outputChannels, gain, *out->buffer, *buffer, offset, frames);

            layer->_chunkPool.Release(out);
            layer->_chunkPool.Release(source);
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
        return layer->instancingEnabled && layer->instancingMode == eChannelInstanceMode_Separate && !layer->instanceData.empty();
    }

    void AmplimixImpl::UpdateSpeed(AmplimixLayerImpl* layer, AmUInt64 frames)
    {
        const AmReal32 target = AM_MAX(AMPLIMIX_LOAD(&layer->pitch) * AMPLIMIX_LOAD(&layer->userPlaySpeed), 0.001f);

        AmReal32 end = layer->currentSpeed;
        if (end != target)
        {
            // Ease toward the target, and settle exactly so the ramp collapses and the resampler is not re-tuned.
            const AmReal32 next = Lerp(0.75f, end, target);
            end = std::abs(next - target) < 1e-6f ? target : next;
        }

        // The ratio is ramped across the block's frames instead of being held at one value for all of them. A target
        // that moves every block -- a glide, Doppler, a rate change -- would otherwise leave the read position a
        // staircase, and a staircase that moves is audible as a click at every block boundary.
        layer->voice->SetSpeedRamp(layer->currentSpeed, end, frames);
        layer->currentSpeed = end;
    }

    void AmplimixImpl::MixVoiceInstances(
        AmplimixLayerImpl* layer, AudioBuffer* buffer, AmUInt64 offset, AmUInt64 frames, AmReal32 gain, AmUInt16 outputChannels)
    {
        Voice* voice = layer->voice;

        SoundChunk* source = layer->_chunkPool.Acquire(frames, voice->GetChannelCount());
        SoundChunk* out = layer->_chunkPool.Acquire(source->frames, outputChannels);

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

            source->buffer->Clear();
            out->buffer->Clear();

            voice->ApplySpeedTo(slot->stream);
            const auto report = voice->Render(*slot, *source->buffer);
            (instancePipeline != nullptr ? instancePipeline : layer->pipeline.get())->Execute(*source->buffer, *out->buffer);
            voice->ApplyGain(*out->buffer);
            MixLayerInstanceWithGain(layer->_mixGain, outputChannels, gain, *out->buffer, *buffer, offset, frames);

            data.cursor = slot->stream.IsFinished() ? layer->end : slot->stream.GetSourcePosition(slot->reader);
            if (report.finished)
                lastFinish = AM_MAX(lastFinish, report.finishedFrame);
            else
                allFinished = false;

            // Write the cursor back through the channel's atomic slots (the game-owned instance containers are never
            // touched from the audio thread), so the game thread can restore it if this layer is recreated. The id
            // check drops stores that race a re-publication.
            if (layer->cursorSlots != nullptr && index < kAmMaxChannelInstances)
            {
                auto& cursorSlot = layer->cursorSlots[index];
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
        layer->_chunkPool.Release(source);

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

    bool AmplimixImpl::Wait(const std::chrono::milliseconds timeout)
    {
        const auto deadline = std::chrono::steady_clock::now() + timeout;

        while (_isMixing.load(std::memory_order_acquire))
        {
            if (std::chrono::steady_clock::now() >= deadline)
            {
                amLogWarning("Amplimix::Wait timed out - forcing continue");
                return false;
            }

            std::this_thread::sleep_for(std::chrono::microseconds(50));
        }

        return true;
    }

#ifdef AM_TESTING
    void AmplimixImpl::SetMixInProgressForTesting(const bool inProgress)
    {
        _isMixing.store(inProgress, std::memory_order_release);
    }
#endif

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

        // The audio thread no longer reads the layer: every stream it still holds is retired, exactly once. The streaming
        // thread outlives the layers (it is stopped before the mixer deinitializes, and frees at once when stopped).
        if (streaming != nullptr)
        {
            for (StreamSource* source : heldStreams)
                streaming->Retire(source);

            for (StreamSource* source : instanceSources)
                streaming->Retire(source);
        }

        heldStreams.clear();
        instanceSources.clear();
        streaming = nullptr;
        stream = nullptr;
        pendingStream = nullptr;
        swapPosted = false;
        hasQueuedSeek = false;
        activationPending = false;

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
        if (processingInstance)
        {
            // Separate mode: an instance with a value of its own is obstructed by it only.
            if (currentInstanceIndex < instanceData.size() && instanceData[currentInstanceIndex].hasObstruction)
                return instanceData[currentInstanceIndex].obstruction;
        }
        else if (hasBlendedObstruction)
        {
            return blendedObstruction;
        }

        return AMPLIMIX_LOAD_RELAXED(&obstruction);
    }

    AmReal32 AmplimixLayerImpl::GetOcclusion() const
    {
        if (processingInstance)
        {
            // Separate mode: an instance with a value of its own is occluded by it only.
            if (currentInstanceIndex < instanceData.size() && instanceData[currentInstanceIndex].hasOcclusion)
                return instanceData[currentInstanceIndex].occlusion;
        }
        else if (hasBlendedOcclusion)
        {
            return blendedOcclusion;
        }

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

    namespace
    {
        // Built at load, so the audio thread neither constructs it nor takes a function-local static's guard.
        const Orientation kZeroOrientation = Orientation::Zero();

        const Orientation& GetFollowedOrientation(const AmplimixLayer& layer)
        {
            const Entity entity = layer.GetEntity();
            return entity.Valid() ? entity.GetOrientation() : kZeroOrientation;
        }
    } // namespace

    const Orientation& AmplimixLayerImpl::GetOrientation() const
    {
        if (processingInstance && currentInstanceIndex < instanceData.size() && instanceData[currentInstanceIndex].hasOrientation)
            return instanceData[currentInstanceIndex].orientation;

        return GetFollowedOrientation(*this);
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
        // While one instance is processed on its own, the layer is not a multi-position source.
        return !processingInstance && instancingEnabled && !instanceData.empty();
    }

    eChannelInstanceMode AmplimixLayerImpl::GetInstancingMode() const
    {
        return instancingMode;
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

    bool AmplimixLayerImpl::HasInstanceOrientation(AmSize index) const
    {
        return index < instanceData.size() && instanceData[index].hasOrientation;
    }

    const Orientation& AmplimixLayerImpl::GetInstanceOrientation(AmSize index) const
    {
        if (index < instanceData.size() && instanceData[index].hasOrientation)
            return instanceData[index].orientation;

        return GetFollowedOrientation(*this);
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

    AmReal32 AmplimixLayerImpl::GetInstanceAttenuationGain(AmSize index) const
    {
        if (index >= instanceData.size())
            return 1.0f;

        return instanceData[index].attenuationGain;
    }

    bool AmplimixLayerImpl::IsSharingPipelineAcrossInstances() const
    {
        return processingInstance && sharedInstancePipeline;
    }

    void AmplimixLayerImpl::UpdateBlendedInstanceValues()
    {
        hasBlendedObstruction = false;
        hasBlendedOcclusion = false;

        // Separate-mode instances are processed one by one, with their own values.
        if (instanceData.empty() || GetInstancingMode() != eChannelInstanceMode_Blended)
            return;

        const Listener listener = GetListener();
        const Attenuation* attenuation = GetAttenuation();

        // The audible gain of an instance, as the blended attenuation computes it, evaluated once for the obstruction
        // and occlusion blends and for the attenuation node. Without attenuation, or without a listener, every
        // instance counts with its weight alone.
        const bool attenuated = attenuation != nullptr && listener.Valid();
        for (auto& instance : instanceData)
        {
            instance.attenuationGain = attenuated ? ComputeInstanceGain(
                                                        *attenuation, listener, instance.location,
                                                        instance.hasOrientation ? &instance.orientation : nullptr)
                                                  : 1.0f;
        }

        const auto gain = [](const InstanceData& instance) -> AmReal32
        {
            return instance.attenuationGain;
        };

        hasBlendedObstruction = BlendInstanceValues(
            instanceData, AMPLIMIX_LOAD_RELAXED(&obstruction), &InstanceData::obstruction, &InstanceData::hasObstruction, gain,
            blendedObstruction);

        hasBlendedOcclusion = BlendInstanceValues(
            instanceData, AMPLIMIX_LOAD_RELAXED(&occlusion), &InstanceData::occlusion, &InstanceData::hasOcclusion, gain,
            blendedOcclusion);
    }

    void AmplimixLayerImpl::UpdateInstanceData()
    {
        previousInstanceData.clear();
        previousInstanceData.swap(instanceData);

        instancingEnabled = false;
        instancingMode = eChannelInstanceMode_Blended;
        cursorSlots = nullptr;
        hasBlendedObstruction = false;
        hasBlendedOcclusion = false;

        if (snd == nullptr || snd->sound == nullptr)
            return;

        const auto& channel = snd->sound->GetChannel();
        if (!channel.Valid())
            return;

        // The only read of the channel: the snapshot, which stays unchanged until the next block.
        const ChannelInstanceSnapshot& snapshot = channel.GetState()->AcquireInstanceSnapshot();

        instancingEnabled = snapshot.instancingEnabled;
        instancingMode = snapshot.instancingMode;
        cursorSlots = snapshot.cursorSlots;

        if (!snapshot.instancingEnabled)
            return;

        instanceData.assign(snapshot.instances.begin(), snapshot.instances.end());

        // Only separate mode plays instances from a cursor of their own.
        if (instancingMode == eChannelInstanceMode_Separate)
            CarryOverInstanceCursors(previousInstanceData, instanceData);

        UpdateBlendedInstanceValues();
    }
} // namespace SparkyStudios::Audio::Amplitude
