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

#include <queue>

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>
#include <SparkyStudios/Audio/Amplitude/Core/Device.h>
#include <SparkyStudios/Audio/Amplitude/Core/Thread.h>
#include <SparkyStudios/Audio/Amplitude/DSP/AudioConverter.h>
#include <SparkyStudios/Audio/Amplitude/Mixer/Amplimix.h>
#include <SparkyStudios/Audio/Amplitude/Mixer/Pipeline.h>

#include <Mixer/SoundData.h>

#include <Utils/miniaudio/miniaudio_utils.h>
#include <Utils/Utils.h>

#include "engine_config_definition_generated.h"

#define _Atomic(X) std::atomic<X>

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
        AmUInt32 id; // playing id
        _Atomic(PlayStateFlag) flag; // state
        _Atomic(AmUInt64) cursor; // cursor
        _Atomic(AmVec2) gain; // gain
        _Atomic(AmReal32) pitch; // pitch
        SoundData* snd; // sound data
        AmUInt64 start, end; // start and end frames

        _Atomic(AmReal32) obstruction; // obstruction factor
        _Atomic(AmReal32) occlusion; // occlusion factor

        _Atomic(AmReal32) userPlaySpeed; // user-defined sound playback speed
        _Atomic(AmReal32) playSpeed; // current sound playback speed
        _Atomic(AmReal32) targetPlaySpeed; // computed (real) sound playback speed
        _Atomic(AmReal32) sampleRateRatio; // sample rate ratio

        AudioConverter* dataConverter; // miniaudio resampler & channel converter

        /**
         * @brief Resets the layer.
         */
        void Reset();

        AmUInt32 GetId() const override;
        AmUInt64 GetStartPosition() const override;
        AmUInt64 GetEndPosition() const override;
        AmUInt64 GetCurrentPosition() const override;
        AmVec2 GetGain() const override;
        AmReal32 GetPitch() const override;
        AmReal32 GetObstruction() const override;
        AmReal32 GetOcclusion() const override;
        AmReal32 GetPlaySpeed() const override;
        AmVec3 GetLocation() const override;
        Entity GetEntity() const override;
        Listener GetListener() const override;
        Channel GetChannel() const override;
        SoundFormat GetSoundFormat() const override;
        eSpatialization GetSpatialization() const override;
        bool IsLoopEnabled() const override;
        bool IsStreamEnabled() const override;
        const Sound* GetSound() const override;
        const EffectInstance* GetEffect() const override;
        AmUInt32 GetSampleRate() const override;
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

        AmUInt64 Mix(AudioBuffer** outBuffer, AmUInt64 frameCount) override;

        AmUInt32 Play(
            SoundData* sound, PlayStateFlag flag, AmReal32 gain, AmReal32 pan, AmReal32 pitch, AmReal32 speed, AmUInt32 id, AmUInt32 layer);

        AmUInt32 PlayAdvanced(
            SoundData* sound,
            PlayStateFlag flag,
            AmReal32 gain,
            AmReal32 pan,
            AmReal32 pitch,
            AmReal32 speed,
            AmUInt64 startFrame,
            AmUInt64 endFrame,
            AmUInt32 id,
            AmUInt32 layer);

        bool SetObstruction(AmUInt32 id, AmUInt32 layer, AmReal32 obstruction);

        bool SetOcclusion(AmUInt32 id, AmUInt32 layer, AmReal32 occlusion);

        bool SetGainPan(AmUInt32 id, AmUInt32 layer, AmReal32 gain, AmReal32 pan);

        bool SetPitch(AmUInt32 id, AmUInt32 layer, AmReal32 pitch);

        bool SetCursor(AmUInt32 id, AmUInt32 layer, AmUInt64 cursor);

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

    private:
        void ExecuteCommands();
        void MixLayer(AmplimixLayerImpl* layer, AudioBuffer* buffer, AmUInt64 frameCount);
        AmplimixLayerImpl* GetLayer(AmUInt32 layer);
        bool ShouldMix(AmplimixLayerImpl* layer);
        void UpdatePitch(AmplimixLayerImpl* layer);
        void LockAudioMutex();
        void UnlockAudioMutex();

        bool _initialized;

        std::queue<MixerCommand> _commandsStack;

        AmMutexHandle _audioThreadMutex;
        std::unordered_map<AmThreadID, bool> _insideAudioThreadMutex;

        AmUInt32 _nextId;
        _Atomic(AmReal32) _masterGain{};
        AmplimixLayerImpl _layers[kAmplimixLayersCount];
        AmUInt64 _remainingFrames;

        AmUniquePtr<MemoryPoolKind::Amplimix, Pipeline> _pipeline = nullptr;

        DeviceDescription _device;

        AudioBuffer _scratchBuffer;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_MIXER_AMPLIMIX_H
