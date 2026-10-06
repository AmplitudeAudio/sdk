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

#pragma once

#ifndef _AM_IMPLEMENTATION_MIXER_AMPLIMIX_H
#define _AM_IMPLEMENTATION_MIXER_AMPLIMIX_H

#include <condition_variable>
#include <memory>
#include <mutex>
#include <vector>

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>
#include <SparkyStudios/Audio/Amplitude/Core/Device.h>
#include <SparkyStudios/Audio/Amplitude/Core/MPSCQueue.h>
#include <SparkyStudios/Audio/Amplitude/Core/Thread.h>
#include <SparkyStudios/Audio/Amplitude/Mixer/Amplimix.h>
#include <SparkyStudios/Audio/Amplitude/Mixer/Pipeline.h>

#include <Core/Playback/ChannelInstanceInternalState.h>
#include <Mixer/SoundData.h>
#include <Mixer/InstancePipelineTable.h>
#include <Mixer/LayerGainMixer.h>
#include <Mixer/Voice/Voice.h>

#include <Utils/miniaudio/miniaudio_utils.h>
#include <Utils/Utils.h>

#include "engine_config_definition_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
    static constexpr AmUInt32 kAmplimixLayersBits = 12;
    static constexpr AmUInt32 kAmplimixLayersCount = (1 << kAmplimixLayersBits);
    static constexpr AmUInt32 kAmplimixLayersMask = (kAmplimixLayersCount - 1);

    class AmplimixImpl;

    class EngineImpl;
    struct EngineInternalState;
    class ChannelInternalState;

    /**
     * @brief The callback to execute when running a mixer command.
     */
    typedef std::function<bool()> MixerCommandCallback;

    /// Lifetime of a mixer layer slot. Live from StartVoice() until Destroy().
    enum class eLayerSlot : AmUInt8
    {
        Free = 0,
        Live,
    };

    /// How StartVoice() starts a sound.
    struct VoiceStartOptions
    {
        bool loop = false;
        AmUInt32 loopCount = 0; ///< Total plays when looping; 0 loops forever.
        AmReal32 gain = 1.0f;
        AmReal32 pitch = 1.0f;
        AmReal32 speed = 1.0f;
        AmUInt64 startFrame = kVoiceAsap; ///< Audio-clock frame of the first sample.
        AmUInt64 startPosition = 0; ///< First source frame.
        AmUInt64 startPositionClock = kVoiceAsap; ///< Audio-clock frame at which startPosition would have been heard;
                                                   ///< the voice advances it to its real start frame.
        AmTime fadeIn = 0.0; ///< Milliseconds; 0 starts at unity.
        AmString faderName; ///< Transport fade curve; empty is linear.
    };

    class AmplimixLayerImpl final : public AmplimixLayer
    {
    public:
        AmUInt32 id = kAmInvalidObjectId; // playing id
        std::atomic<eLayerSlot> slot{ eLayerSlot::Free }; // Live from StartVoice() until Destroy()
        std::atomic<bool> detached{ false }; // set by the audio thread once it no longer touches the layer
        Voice* voice = nullptr; // created by StartVoice() (game thread), deleted by Destroy()
        AmReal32 currentSpeed = 1.0f; // smoothed pitch x play speed, audio thread
        std::atomic<eVoiceState> voiceState{ eVoiceState::Idle }; // voice state after the last block, for any thread
        std::atomic<AmUInt64> voicePosition{ 0 }; // voice source position after the last block, for any thread
        std::atomic<AmReal32> gain; // gain
        std::atomic<AmReal32> pitch; // pitch
        SoundData* snd = nullptr; // sound data
        AmUInt64 start = 0, end = 0; // start and end frames

        std::atomic<AmReal32> obstruction; // obstruction factor
        std::atomic<AmReal32> occlusion; // occlusion factor

        std::atomic<AmReal32> userPlaySpeed; // user-defined sound playback speed

        std::shared_ptr<PipelineInstance> pipeline = nullptr; // pipeline for this layer

        SoundChunkPool _chunkPool; // pool for reusable SoundChunk allocations
        std::shared_ptr<InstancePipelineTable> instancePipelines; // per-instance pipelines (separate mode), set by mixer commands
        std::shared_ptr<InstanceStreamTable> instanceStreams; // per-instance voice streams (separate mode), set by mixer commands
        GainProcessor _mixGain[kAmplimixMaxOutputChannels]; // master x layer gain ramps, audio-thread owned after publication
        AmUInt64 pipelineFrames = 0; // frames per-instance pipelines are pre-configured with, set by StartVoice
        std::atomic<AmUInt32> pins{ 0 }; // game-thread readers keeping the layer alive, see AmplimixImpl::PinLayer
        std::atomic<bool> destroyPending{ false }; // the game thread is destroying the layer: pins fail, see PinLayer
        bool releaseRequested = false; // game thread only: a release or discard was pushed, the layer may be destroyed

        ~AmplimixLayerImpl() override;

        /**
         * @brief Destroys the layer's pipeline and sound reference (game thread, or the mixer shutdown).
         *
         * Releases the voice, the pipeline instance and the sound data pointer,
         * then frees the layer slot. Safe to call even if the layer has no sound attached.
         */
        void Destroy();

        /**
         * @brief Resets the pipeline to its initial state.
         *
         * This clears the pipeline's per-block caches and the room update flag; node DSP state persists.
         * Should be called between instance processing in separate mode, when instances share this pipeline.
         */
        void ResetPipeline();

        /**
         * @brief Clears the room update flag so the next instance can re-initialize room handling.
         */
        void ResetRoomUpdateFlag();

        [[nodiscard]] AmUInt32 GetId() const override;
        [[nodiscard]] AmUInt64 GetStartPosition() const override;
        [[nodiscard]] AmUInt64 GetEndPosition() const override;
        [[nodiscard]] AmUInt64 GetCurrentPosition() const override;
        [[nodiscard]] AmReal32 GetGain() const override;
        [[nodiscard]] AmReal32 GetPitch() const override;
        [[nodiscard]] AmReal32 GetObstruction() const override;
        [[nodiscard]] AmReal32 GetOcclusion() const override;
        [[nodiscard]] AmReal32 GetPlaySpeed() const override;
        [[nodiscard]] AmVector3 GetLocation() const override;
        [[nodiscard]] Entity GetEntity() const override;
        [[nodiscard]] Listener GetListener() const override;
        [[nodiscard]] Room GetRoom() const override;
        [[nodiscard]] Channel GetChannel() const override;
        [[nodiscard]] Bus GetBus() const override;
        [[nodiscard]] SoundFormat GetSoundFormat() const override;
        [[nodiscard]] eSpatialization GetSpatialization() const override;
        [[nodiscard]] bool IsLoopEnabled() const override;
        [[nodiscard]] bool IsStreamEnabled() const override;
        [[nodiscard]] const Sound* GetSound() const override;
        [[nodiscard]] const std::shared_ptr<EffectInstance> GetEffect() const override;
        [[nodiscard]] const Attenuation* GetAttenuation() const override;
        [[nodiscard]] AmUInt32 GetSampleRate() const override;
        [[nodiscard]] bool IsMultiPosition() const override;
        [[nodiscard]] eChannelInstanceMode GetInstancingMode() const override;
        [[nodiscard]] AmSize GetInstanceCount() const override;
        [[nodiscard]] AmVector3 GetInstanceLocation(AmSize index) const override;
        [[nodiscard]] Room GetInstanceRoom(AmSize index) const override;
        [[nodiscard]] AmReal32 GetInstanceWeight(AmSize index) const override;
        [[nodiscard]] AmReal32 GetInstanceGain(AmSize index) const override;
        [[nodiscard]] bool IsSharingPipelineAcrossInstances() const override;

        /**
         * @brief Cached per-instance data for pipeline processing.
         *
         * Alias of the Core snapshot record; see @c ChannelInstanceData.
         */
        using InstanceData = ChannelInstanceData;

        /**
         * @brief Updates the cached instance data from the channel's published snapshot.
         *
         * Reads the immutable snapshot published by the game thread (never the
         * game-owned containers), preserving layer-side cursors for instances whose
         * id was already present.
         */
        void UpdateInstanceData();

        /**
         * @brief Cached instance data for multi-position processing.
         */
        std::vector<InstanceData> instanceData;

        /**
         * @brief Scratch storage used by UpdateInstanceData() to preserve cursors
         * across snapshot refreshes without allocating on the audio thread.
         */
        std::vector<InstanceData> previousInstanceData;

        /**
         * @brief Whether we are currently processing a specific instance (separate mode).
         *
         * When true, IsMultiPosition() returns false and GetLocation() returns
         * the current instance's location.
         */
        bool processingInstance = false;

        /**
         * @brief Whether the instance being processed (separate mode) renders through the shared layer pipeline.
         *
         * True for instances without their own pipeline (overflow, or per-instance pipelines disabled).
         */
        bool sharedInstancePipeline = false;

        /**
         * @brief The index of the current instance being processed (separate mode).
         */
        AmSize currentInstanceIndex = 0;
    };

    struct MixerCommand
    {
        MixerCommandCallback callback; // command callback
    };

    /**
     * @brief Amplimix - The Amplitude Audio Mixer
     */
    class AmplimixImpl final : public Amplimix
    {
        friend struct AmplimixMutexLocker;

    public:
        explicit AmplimixImpl(AmReal32 masterGain);

        ~AmplimixImpl() override;

        /**
         * @brief Initializes the audio Mixer.
         *
         * @param config The audio engine configuration.
         * @return true on success, false on failure.
         */
        bool Init(const EngineConfigDefinition* config);

        /**
         * @brief Deinitializes the audio mixer.
         */
        void Deinit();

        /**
         * @copydoc Amplimix::UpdateDevice
         */
        void UpdateDevice(
            AmObjectID deviceID,
            AmString deviceName,
            AmUInt32 deviceOutputSampleRate,
            PlaybackOutputChannels deviceOutputChannels,
            PlaybackOutputFormat deviceOutputFormat) override;

        [[nodiscard]] AM_INLINE bool IsInitialized() const override
        {
            return _initialized;
        }

        void SetAfterMixCallback(AfterMixCallback callback) override;

        AmUInt64 Mix(AudioBuffer** outBuffer, AmUInt64 frameCount) override;

        /**
         * @brief Starts @p sound on a new voice (game thread).
         *
         * @param sound The sound data to render. The layer owns it from here until the layer is destroyed.
         * @param options How the voice starts.
         * @param id The owner id checked by every later call on the layer.
         * @param layer The layer handle to use, or 0 to allocate one.
         *
         * @return The layer handle, or 0 if the voice could not start.
         */
        AmUInt32 StartVoice(SoundData* sound, const VoiceStartOptions& options, AmUInt32 id, AmUInt32 layer);

        /**
         * @brief Records a transport command for a voice (game thread). Sent by the next @c FlushVoiceCommands().
         */
        void PostVoiceCommand(
            AmUInt32 id, AmUInt32 layer, eVoiceCommandKind kind, AmTime duration = 0.0, AmUInt64 position = 0, AmUInt64 frame = kVoiceAsap);

        /**
         * @brief Sends the recorded voice commands to the audio thread (game thread, once per frame).
         *
         * Commands that do not fit are kept for the next flush: nothing is dropped.
         */
        void FlushVoiceCommands();

        /**
         * @brief Handles the events published by voices and fires the matching channel callbacks (game thread).
         */
        void DispatchVoiceEvents();

        /**
         * @brief Gets the state last published by the voice of a layer.
         *
         * @return @c eVoiceState::Idle when the layer is gone or not playing for @p id.
         */
        [[nodiscard]] eVoiceState GetVoiceState(AmUInt32 id, AmUInt32 layer) const;

        /**
         * @brief Gets the source position last published by the voice of a layer.
         *
         * @return @c false when the layer is gone or not playing for @p id.
         */
        [[nodiscard]] bool GetVoicePosition(AmUInt32 id, AmUInt32 layer, AmUInt64& position) const;

        /**
         * @brief Gets the number of output frames rendered since the mixer was initialized.
         */
        [[nodiscard]] AmUInt64 GetAudioClock() const;

        /**
         * @brief Destroys a layer whose voice finished, after the current mix (game thread).
         */
        void ReleaseLayer(AmUInt32 id, AmUInt32 layer);

        /**
         * @brief Destroys a layer without waiting for its voice to finish, after the current mix (game thread).
         */
        void DiscardVoice(AmUInt32 id, AmUInt32 layer);

        /**
         * @brief Gives a separate-mode instance its own voice stream, starting at the instance cursor (game thread).
         *
         * @return @c false if the layer is not playing for @p id or the stream could not be created.
         */
        bool AttachInstanceStream(AmUInt32 id, AmUInt32 layer, AmChannelInstanceID instanceId);

        /**
         * @brief Removes the voice stream of a separate-mode instance; it is released after the mix (game thread).
         *
         * @return @c false if the layer is not playing for @p id.
         */
        bool DetachInstanceStream(AmUInt32 id, AmUInt32 layer, AmChannelInstanceID instanceId);

        bool SetObstruction(AmUInt32 id, AmUInt32 layer, AmReal32 obstruction);

        bool SetOcclusion(AmUInt32 id, AmUInt32 layer, AmReal32 occlusion);

        bool SetGain(AmUInt32 id, AmUInt32 layer, AmReal32 gain);

        bool SetPitch(AmUInt32 id, AmUInt32 layer, AmReal32 pitch);

        bool SetPlaySpeed(AmUInt32 id, AmUInt32 layer, AmReal32 speed);

        void SetMasterGain(AmReal32 gain);

        /**
         * @brief Posts a stop to every live voice (game thread).
         */
        void StopAll();

        /**
         * @brief Posts a pause to every live voice (game thread).
         */
        void HaltAll();

        /**
         * @brief Posts a resume to every live voice (game thread).
         */
        void PlayAll();

        [[nodiscard]] bool IsInsideThreadMutex() const;

        void PushCommand(const MixerCommand& command);

        /**
         * @brief Default value of the engine config @c mixer.max_instance_pipelines field.
         */
        static constexpr AmSize kDefaultMaxInstancePipelines = 32;

        /**
         * @brief Gets the maximum number of per-instance pipelines a separate-mode layer may own.
         */
        [[nodiscard]] AmSize GetMaxInstancePipelines() const;

        /**
         * @brief Sets the maximum number of per-instance pipelines a separate-mode layer may own. 0 disables them.
         */
        void SetMaxInstancePipelines(AmSize count);

        /**
         * @brief Gives a playing layer empty instance pipeline and instance stream tables (game thread).
         *
         * @return @c false if the layer is not playing for @p id.
         */
        bool InstallInstancePipelineTable(AmUInt32 id, AmUInt32 layer);

        /**
         * @brief Creates a pipeline for @p instanceId on the game thread and attaches it to the layer.
         *
         * @return @c false if the layer is not playing for @p id or no pipeline could be created.
         */
        bool AttachInstancePipeline(AmUInt32 id, AmUInt32 layer, AmChannelInstanceID instanceId);

        /**
         * @brief Detaches the pipeline of @p instanceId from the layer; it is released after the mix.
         *
         * @return @c false if the layer is not playing for @p id.
         */
        bool DetachInstancePipeline(AmUInt32 id, AmUInt32 layer, AmChannelInstanceID instanceId);

        /**
         * @brief Gets the number of per-instance pipelines attached to a layer playing for @p id.
         *
         * Diagnostic accessor for tests: reads audio-thread-owned state without synchronization; call only after
         * the mixer has run the pending commands (e.g. after Engine::WaitUntilFrames).
         *
         * @return The number of attached pipelines, or 0 if the layer is not playing for @p id or has no table.
         */
        [[nodiscard]] AmSize GetInstancePipelineCount(AmUInt32 id, AmUInt32 layer) const;

        /**
         * @brief Keeps a playing layer from being destroyed while the game thread reads it outside the audio thread's control.
         *
         * A destroy requested while the layer is pinned is deferred until the last @c UnpinLayer().
         *
         * @return @c false if the layer is not playing for @p id; the layer is not pinned then.
         */
        [[nodiscard]] bool PinLayer(AmUInt32 id, AmUInt32 layer);

        /**
         * @brief Releases a pin taken by a successful @c PinLayer(), running any destroy it deferred.
         */
        void UnpinLayer(AmUInt32 layer);

        [[nodiscard]] const Pipeline* GetPipeline() const;

        [[nodiscard]] Pipeline* GetPipeline();

        [[nodiscard]] AM_INLINE const DeviceDescription& GetDeviceDescription() const override
        {
            return _device;
        }

        static void IncrementSoundLoopCount(SoundInstance* sound);

        AmUInt32 GetLayerIndex(const AmplimixLayerImpl* layer) const;
        void ActivateLayer(AmUInt32 layerIndex);
        void DeactivateLayer(AmUInt32 layerIndex);

    private:
        friend class EngineImpl;

        void ExecuteCommands();
        void DestroyDetachedLayers();
        bool TryDestroyLayer(AmplimixLayerImpl* layer);
        void UpdateVoiceBlockFrames();
        void DrainVoiceCommands();
        void HandleVoiceEvent(const VoiceEvent& event);
        void HandleVoiceEnded(AmplimixLayerImpl* layer, ChannelInternalState* channelState);
        bool MixVoice(AmplimixLayerImpl* layer, AudioBuffer* buffer, AmUInt64 offset, AmUInt64 frames, AmUInt64 blockClock);
        void MixVoiceInstances(
            AmplimixLayerImpl* layer, AudioBuffer* buffer, AmUInt64 offset, AmUInt64 frames, AmReal32 gain, AmUInt16 outputChannels);
        [[nodiscard]] static bool IsSeparateMode(const AmplimixLayerImpl* layer);
        static void UpdateSpeed(AmplimixLayerImpl* layer, AmUInt64 frames);
        AmplimixLayerImpl* GetLayer(AmUInt32 layer);
        bool ShouldMix(AmplimixLayerImpl* layer);
        void LockAudioMutex();
        void UnlockAudioMutex();
        void Wait();

        // Cached engine state for the audio thread. Engine::GetInstance() locks a mutex
        // and GetState() copies a shared_ptr; neither may run in the mix callback.
        // Valid from Init() until Deinit(); the engine state outlives the mixer by
        // deinitialization order.
        EngineInternalState* _engineState = nullptr;

        bool _initialized;

        AmString _resamplerName;

        MPSCQueue<MixerCommand, kAmplimixLayersCount> _commandsStack;

        std::recursive_timed_mutex _audioThreadMutex;

        std::atomic<bool> _isMixing{ false };
        std::mutex _mixCompleteMutex;
        std::condition_variable _mixCompleteCV;

        AmUInt32 _nextId;
        std::atomic<AmReal32> _masterGain{};
        AmplimixLayerImpl _layers[kAmplimixLayersCount];
        AmUInt32 _activeLayerIndices[kAmplimixLayersCount];
        std::atomic<AmUInt32> _activeLayerCount = 0;

        std::unique_ptr<VoiceCommandQueue> _voiceCommands;
        std::unique_ptr<VoiceEventQueue> _voiceEvents;
        std::vector<VoiceCommand> _voiceOutbox; // game side only, guarded by _voiceOutboxMutex
        std::vector<AmUInt32> _pendingDestroys; // game thread only: released layers waiting for detach and unpin
        std::mutex _voiceOutboxMutex; // transport may be called from any game-side thread; never taken by the audio thread
        std::atomic<AmUInt64> _audioClock{ 0 };
        AmUInt64 _voiceBlockFrames = 0;

        Pipeline* _pipeline = nullptr;
        AmSize _maxInstancePipelines = kDefaultMaxInstancePipelines;

        DeviceDescription _device;

        // Atomic snapshots of device fields read by the audio thread.
        std::atomic<AmUInt32> _mixOutputSampleRate{};
        std::atomic<PlaybackOutputChannels> _mixOutputChannels{};

        AudioBuffer _scratchBuffer;

        AfterMixCallback _afterMixCallback = nullptr;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_MIXER_AMPLIMIX_H
