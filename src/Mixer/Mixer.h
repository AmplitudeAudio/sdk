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

#ifndef SS_AMPLITUDE_AUDIO_MIXER_H
#define SS_AMPLITUDE_AUDIO_MIXER_H

#include <queue>

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>
#include <SparkyStudios/Audio/Amplitude/Core/Device.h>
#include <SparkyStudios/Audio/Amplitude/Core/Thread.h>
#include <SparkyStudios/Audio/Amplitude/Mixer/Resampler.h>

#include <Mixer/ProcessorPipeline.h>
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

    class Mixer;

    /**
     * @brief Called just before the mixer process audio data.
     */
    typedef void (*BeforeMixCallback)(Mixer* mixer, AmAudioFrameBuffer audio, AmUInt32 frames);

    /**
     * @brief Called just after the mixer process audio data.
     */
    typedef void (*AfterMixCallback)(Mixer* mixer, AmAudioFrameBuffer audio, AmUInt32 frames);

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

    struct MixerLayer
    {
        BeforeMixCallback onBeforeMix; // called before the mixing
        AfterMixCallback onAfterMix; // called after the mixing

        AmUInt32 id; // playing id
        _Atomic(PlayStateFlag) flag; // state
        _Atomic(AmUInt64) cursor; // cursor
        _Atomic(AmVec2) gain; // gain
        _Atomic(AmReal32) pitch; // pitch
        SoundData* snd; // sound data
        AmUInt64 start, end; // start and end frames

        _Atomic(AmReal32) userPlaySpeed; // user-defined sound playback speed
        _Atomic(AmReal32) playSpeed; // computed (real) sound playback speed
        _Atomic(AmReal32) sampleRateRatio; // sample rate

        ma_data_converter dataConverter; // miniaudio resampler

        void Reset();
    };

    struct MixerCommand
    {
        MixerCommandCallback callback; // command callback
    };

    /**
     * @brief Amplimix - The Amplitude Audio Mixer
     */
    class Mixer
    {
    public:
        explicit Mixer(AmReal32 masterGain);

        ~Mixer();

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
         * @brief Mixer post initialization.
         *
         * This method is called once, just after the playback device is initialized
         * and before it is started.
         *
         * @param bufferSize The buffer size accepted by the playback device.
         * @param sampleRate The sample rate accepted bu the playback device.
         * @param channels The number of channels the playback device will output.
         */
        void UpdateDevice(
            AmObjectID deviceID,
            AmString deviceName,
            AmUInt32 deviceOutputSampleRate,
            PlaybackOutputChannels deviceOutputChannels,
            PlaybackOutputFormat deviceOutputFormat);

        [[nodiscard]] bool IsInitialized() const;

        AmUInt64 Mix(AmVoidPtr mixBuffer, AmUInt64 frameCount);

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

        [[nodiscard]] const ProcessorPipeline* GetPipeline() const;

        [[nodiscard]] ProcessorPipeline* GetPipeline();

        [[nodiscard]] const DeviceDescription& GetDeviceDescription() const;

        static void IncrementSoundLoopCount(SoundInstance* sound);

    private:
        void ExecuteCommands();
        void MixLayer(MixerLayer* layer, AmAudioFrameBuffer buffer, AmUInt64 bufferSize, AmUInt64 samples);
        MixerLayer* GetLayer(AmUInt32 layer);
        bool ShouldMix(MixerLayer* layer);
        void UpdatePitch(MixerLayer* layer);
        void LockAudioMutex();
        void UnlockAudioMutex();

        bool _initialized;

        std::queue<MixerCommand> _commandsStack;

        AmMutexHandle _audioThreadMutex;
        std::unordered_map<AmThreadID, bool> _insideAudioThreadMutex;

        AmUInt32 _nextId;
        _Atomic(AmReal32) _masterGain{};
        MixerLayer _layers[kAmplimixLayersCount];
        AmUInt64 _remainingFrames;

        ProcessorPipeline* _pipeline;

        DeviceDescription _device;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_MIXER_H
