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

#include <atomic>
#include <queue>
#define _Atomic(X) std::atomic<X>

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

#include <Mixer/SoundData.h>
#include <Utils/SmMalloc/smmalloc.h>

#include "engine_config_definition_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
    static constexpr AmUInt32 kAmplimixLayersBits = 12;
    static constexpr AmUInt32 kAmplimixLayersCount = (1 << kAmplimixLayersBits);
    static constexpr AmUInt32 kAmplimixLayersMask = (kAmplimixLayersCount - 1);

    class Mixer;

    /**
     * @brief Called just before the mixer process audio data.
     */
    typedef void (*BeforeMixCallback)(Mixer* mixer, AudioBuffer audio, AmUInt32 frames);

    /**
     * @brief Called just after the mixer process audio data.
     */
    typedef void (*AfterMixCallback)(Mixer* mixer, AudioBuffer audio, AmUInt32 frames);

    /**
     * @brief The callback to execute when running a mixer command.
     */
    typedef std::function<bool()> MixerCommandCallback;

    enum PlayStateFlag : AmUInt8
    {
        PLAY_STATE_FLAG_MIN = 0,
        PLAY_STATE_FLAG_STOP = 1,
        PLAY_STATE_FLAG_HALT = 2,
        PLAY_STATE_FLAG_PLAY = 3,
        PLAY_STATE_FLAG_LOOP = 4,
        PLAY_STATE_FLAG_MAX,
    };

    struct MixerLayer
    {
        BeforeMixCallback onBeforeMix; // called before the mixing
        AfterMixCallback onAfterMix; // called after the mixing

        AmUInt32 id; // playing id
        _Atomic(PlayStateFlag) flag; // state
        _Atomic(AmUInt64) cursor; // cursor
        _Atomic(hmm_vec2) gain; // gain
        SoundData* snd; // sound data
        AmUInt64 start, end; // start and end frames
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
        explicit Mixer(float masterGain);

        ~Mixer();

        /**
         * @brief Initializes the audio Mixer.
         *
         * @param config The audio engine configuration.
         * @return true on success, false on failure.
         */
        bool Init(const EngineConfigDefinition* config);

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
        void PostInit(AmUInt32 bufferSize, AmUInt32 sampleRate, AmUInt16 channels);

        AmUInt64 Mix(AmVoidPtr mixBuffer, AmUInt64 frameCount);

        AmUInt32 Play(SoundData* sound, PlayStateFlag flag, float gain, float pan, AmUInt32 id, AmUInt32 layer);

        AmUInt32 PlayAdvanced(
            SoundData* sound,
            PlayStateFlag flag,
            float gain,
            float pan,
            AmUInt64 startFrame,
            AmUInt64 endFrame,
            AmUInt32 id,
            AmUInt32 layer);

        bool SetGainPan(AmUInt32 id, AmUInt32 layer, float gain, float pan);

        bool SetCursor(AmUInt32 id, AmUInt32 layer, AmUInt64 cursor);

        bool SetPlayState(AmUInt32 id, AmUInt32 layer, PlayStateFlag flag);

        PlayStateFlag GetPlayState(AmUInt32 id, AmUInt32 layer);

        void SetMasterGain(float gain);

        void StopAll();

        void HaltAll();

        void PlayAll();

        [[nodiscard]] bool IsInsideThreadMutex() const;

        void PushCommand(const MixerCommand& command);

    private:
        void ExecuteCommands();
        void OnSoundStarted(const SoundData* data);
        void OnSoundPaused(const SoundData* data);
        void OnSoundResumed(const SoundData* data);
        void OnSoundStopped(const SoundData* data);
        bool OnSoundLooped(const SoundData* data);
        AmUInt64 OnSoundStream(const SoundData* data, AmUInt64 offset, AmUInt64 frames);
        void OnSoundEnded(const SoundData* data);
        void OnSoundDestroyed(const SoundData* data);
        void MixLayer(MixerLayer* layer, AudioBuffer buffer, AmUInt64 bufferSize, AmUInt64 samples);
        AmUInt64 MixMono(
            MixerLayer* layer,
            bool loop,
            AmUInt64 cursor,
            AudioDataUnit lGain,
            AudioDataUnit rGain,
            AudioBuffer buffer,
            AmUInt64 bufferSize);
        AmUInt64 MixStereo(
            MixerLayer* layer,
            bool loop,
            AmUInt64 cursor,
            AudioDataUnit lGain,
            AudioDataUnit rGain,
            AudioBuffer buffer,
            AmUInt64 bufferSize);
        MixerLayer* GetLayer(AmUInt32 layer);
        bool ShouldMix(MixerLayer* layer);
        void LockAudioMutex();
        void UnlockAudioMutex();

        bool _initialized;

        std::queue<MixerCommand> _commandsStack;

        AmVoidPtr _audioThreadMutex;
        bool _insideAudioThreadMutex;

        AmUInt32 _requestedBufferSize;
        AmUInt32 _requestedSampleRate;
        AmUInt16 _requestedChannels;

        AmUInt32 _deviceBufferSize;
        AmUInt32 _deviceSampleRate;
        AmUInt16 _deviceChannels;

        AmUInt32 _nextId;
        _Atomic(float) _masterGain;
        MixerLayer _layers[kAmplimixLayersCount];
        AmUInt64 _remainingFrames;

        sm_allocator _mixBufferAllocator;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_MIXER_H
