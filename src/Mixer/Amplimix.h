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
#include <mutex>

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>
#include <SparkyStudios/Audio/Amplitude/Core/Device.h>
#include <SparkyStudios/Audio/Amplitude/Core/MPSCQueue.h>
#include <SparkyStudios/Audio/Amplitude/Core/Thread.h>
#include <SparkyStudios/Audio/Amplitude/DSP/AudioConverter.h>
#include <SparkyStudios/Audio/Amplitude/Mixer/Amplimix.h>
#include <SparkyStudios/Audio/Amplitude/Mixer/Pipeline.h>

#include <Mixer/SoundData.h>

#include <Utils/miniaudio/miniaudio_utils.h>
#include <Utils/Utils.h>

#include "engine_config_definition_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
    static constexpr AmUInt32 kAmplimixLayersBits = 12;
    static constexpr AmUInt32 kAmplimixLayersCount = (1 << kAmplimixLayersBits);
    static constexpr AmUInt32 kAmplimixLayersMask = (kAmplimixLayersCount - 1);

    class AmplimixImpl;

    /**
     * @brief The callback to execute when running a mixer command.
     */
    typedef std::function<bool()> MixerCommandCallback;

    enum PlayStateFlag : AmUInt8
    {
        ePSF_MIN = 0,
        ePSF_STOP = 1,
        ePSF_HALT = 2,
        ePSF_PLAY = 3,
        ePSF_LOOP = 4,
        ePSF_MAX,
    };

    class AmplimixLayerImpl final : public AmplimixLayer
    {
    public:
        AmUInt32 id = kAmInvalidObjectId; // playing id
        std::atomic<PlayStateFlag> flag; // state
        std::atomic<AmUInt64> cursor; // cursor
        std::atomic<AmReal32> gain; // gain
        std::atomic<AmReal32> pitch; // pitch
        SoundData* snd = nullptr; // sound data
        AmUInt64 start = 0, end = 0; // start and end frames

        std::atomic<AmReal32> obstruction; // obstruction factor
        std::atomic<AmReal32> occlusion; // occlusion factor

        std::atomic<AmReal32> userPlaySpeed; // user-defined sound playback speed
        std::atomic<AmReal32> playSpeed; // current sound playback speed
        std::atomic<AmReal32> targetPlaySpeed; // computed (real) sound playback speed
        std::atomic<AmReal32> sampleRateRatio; // sample rate ratio
        std::atomic<AmReal32> baseSampleRateRatio; // base sample rate ratio

        AudioConverter* dataConverter = nullptr; // miniaudio resampler & channel converter
        std::shared_ptr<PipelineInstance> pipeline = nullptr; // pipeline for this layer

        SoundChunkPool _chunkPool; // pool for reusable SoundChunk allocations

        ~AmplimixLayerImpl() override;

        /**
         * @brief Destroys the layer's pipeline and sound reference.
         *
         * Releases the pipeline instance and the sound data pointer,
         * then sets the layer flag to ePSF_MIN. Safe to call even if
         * the layer has no sound attached.
         */
        void Destroy();

        /**
         * @brief Resets the pipeline to its initial state.
         *
         * This clears all stateful nodes (filters, reverbs, etc.) and resets
         * room update flags. Should be called between instance processing in
         * separate mode to ensure independent processing per spatial position.
         */
        void ResetPipeline();

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

        /**
         * @brief Cached per-instance data for pipeline processing.
         */
        struct InstanceData
        {
            AmChannelInstanceID instanceId;
            AmVector3 location;
            Room room;
            AmReal32 weight;
            AmReal32 computedGain;
            AmUInt64 cursor; // For separate mode
        };

        /**
         * @brief Updates the cached instance data from the channel state.
         */
        void UpdateInstanceData();

        /**
         * @brief Cached instance data for multi-position processing.
         */
        std::vector<InstanceData> instanceData;

        /**
         * @brief Whether we are currently processing a specific instance (separate mode).
         *
         * When true, IsMultiPosition() returns false and GetLocation() returns
         * the current instance's location.
         */
        bool processingInstance = false;

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

        AmUInt32 Play(SoundData* sound, PlayStateFlag flag, AmReal32 gain, AmReal32 pitch, AmReal32 speed, AmUInt32 id, AmUInt32 layer);

        AmUInt32 PlayAdvanced(
            SoundData* sound,
            PlayStateFlag flag,
            AmReal32 gain,
            AmReal32 pitch,
            AmReal32 speed,
            AmUInt64 startFrame,
            AmUInt64 endFrame,
            AmUInt32 id,
            AmUInt32 layer);

        bool SetObstruction(AmUInt32 id, AmUInt32 layer, AmReal32 obstruction);

        bool SetOcclusion(AmUInt32 id, AmUInt32 layer, AmReal32 occlusion);

        bool SetGain(AmUInt32 id, AmUInt32 layer, AmReal32 gain);

        bool SetPitch(AmUInt32 id, AmUInt32 layer, AmReal32 pitch);

        bool SetCursor(AmUInt32 id, AmUInt32 layer, AmUInt64 cursor);

        bool GetCursor(AmUInt32 id, AmUInt32 layer, AmUInt64& cursor);

        bool ResetLayerState(AmUInt32 id, AmUInt32 layer);

        bool SetPlayState(AmUInt32 id, AmUInt32 layer, PlayStateFlag flag);

        bool SetPlaySpeed(AmUInt32 id, AmUInt32 layer, AmReal32 speed);

        PlayStateFlag GetPlayState(AmUInt32 id, AmUInt32 layer);

        void SetMasterGain(AmReal32 gain);

        void StopAll();

        void HaltAll();

        void PlayAll();

        [[nodiscard]] bool IsInsideThreadMutex() const;

        void PushCommand(const MixerCommand& command);

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
        void MixLayer(AmplimixLayerImpl* layer, AudioBuffer* buffer, AmUInt64 frameCount);
        void MixLayerSeparateMode(AmplimixLayerImpl* layer, AudioBuffer* buffer, AmUInt64 frameCount);
        AmplimixLayerImpl* GetLayer(AmUInt32 layer);
        bool ShouldMix(AmplimixLayerImpl* layer);
        void UpdatePitch(AmplimixLayerImpl* layer);
        void LockAudioMutex();
        void UnlockAudioMutex();
        void Wait();

        bool _initialized;

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
        AmUInt64 _remainingFrames;

        Pipeline* _pipeline = nullptr;

        DeviceDescription _device;

        // Atomic snapshots of device fields read by the audio thread.
        std::atomic<AmUInt32> _mixOutputSampleRate{};
        std::atomic<PlaybackOutputChannels> _mixOutputChannels{};

        AudioBuffer _scratchBuffer;

        AfterMixCallback _afterMixCallback = nullptr;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_MIXER_AMPLIMIX_H
