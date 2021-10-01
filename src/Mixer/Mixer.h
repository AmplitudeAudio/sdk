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

#ifndef __cplusplus
#include <stdatomic.h> //atomics
#else
#include <atomic> //atomics
#define _Atomic(X) std::atomic<X>
#endif
#include <cstring> //memcpy

#define AMPLIMIX_LBITS 16
#define AMPLIMIX_LAYERS (1 << AMPLIMIX_LBITS)
#define AMPLIMIX_LMASK (AMPLIMIX_LAYERS - 1)

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

#include <Mixer/SoundData.h>

#include "engine_config_definition_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
    class Mixer;

    /**
     * @brief Called just before the mixer process audio data.
     */
    typedef void (*BeforeMixCallback)(Mixer* mixer, AudioBuffer audio, AmUInt32 frames);

    /**
     * @brief Called just after the mixer process audio data.
     */
    typedef void (*AfterMixCallback)(Mixer* mixer, AudioBuffer audio, AmUInt32 frames);

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
        _Atomic(AmInt32) cursor; // cursor
        _Atomic(hmm_vec2) gain; // gain
        SoundData* snd; // sound data
        AmInt32 start, end; // start and end frames
    };

    /**
     * @brief Amplitude Mixer
     */
    class Mixer
    {
    public:
        /**
         * @brief Defines the SoundEndedCallback to use in the mixer.
         *
         * @param callback The sound ended callback.
         */

        Mixer(float masterGain);

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
         */
        void PostInit(AmUInt32 bufferSize, AmUInt32 sampleRate, AmUInt32 channels);

        AmUInt32 Mix(AmVoidPtr mixBuffer, AmUInt32 frameCount);

        AmUInt32 Play(SoundData* sound, PlayStateFlag flag, float gain, float pan, AmUInt32 id, AmUInt32 layer);

        AmUInt32 PlayAdvanced(
            SoundData* sound, PlayStateFlag flag, float gain, float pan, AmInt32 startFrame, AmInt32 endFrame, AmUInt32 id, AmUInt32 layer);

        bool SetGainPan(AmUInt32 id, AmUInt32 layer, float gain, float pan);

        bool SetCursor(AmUInt32 id, AmUInt32 layer, AmInt32 cursor);

        bool SetPlayState(AmUInt32 id, AmUInt32 layer, PlayStateFlag flag);

        PlayStateFlag GetPlayState(AmUInt32 id, AmUInt32 layer);

        void SetMasterGain(float gain);

        void StopAll();

        void HaltAll();

        void PlayAll();

    private:
        void OnSoundStarted(SoundData* data);
        void OnSoundPaused(SoundData* data);
        void OnSoundResumed(SoundData* data);
        void OnSoundStopped(SoundData* data);
        bool OnSoundLooped(SoundData* data);
        AmUInt64 OnSoundStream(SoundData* sound, AmUInt64 offset, AmUInt64 frames);
        void OnSoundEnded(SoundData* snd);
        void OnSoundDestroyed(SoundData* snd);
        void MixLayer(MixerLayer* layer, AudioBuffer buffer, AmUInt32 bufferSize, AmUInt32 samples);
        AmInt32 MixMono(
            MixerLayer* layer,
            bool loop,
            AmInt32 cursor,
            AudioDataUnit lGain,
            AudioDataUnit rGain,
            AudioBuffer buffer,
            AmUInt32 bufferSize);
        AmInt32 MixStereo(
            MixerLayer* layer,
            bool loop,
            AmInt32 cursor,
            AudioDataUnit lGain,
            AudioDataUnit rGain,
            AudioBuffer buffer,
            AmUInt32 bufferSize);
        MixerLayer* GetLayer(AmUInt32 layer);
        void LockAudioMutex();
        void UnlockAudioMutex();

        bool _initialized;

        AmVoidPtr _audioThreadMutex;
        bool _insideAudioThreadMutex;

        AmUInt32 _requestedBufferSize;
        AmUInt32 _requestedSampleRate;
        AmUInt32 _requestedChannels;

        AmUInt32 _deviceBufferSize;
        AmUInt32 _deviceSampleRate;
        AmUInt32 _deviceChannels;

        AmUInt32 _nextId;
        _Atomic(float) _masterGain;
        MixerLayer _layers[AMPLIMIX_LAYERS];
        AmInt32 _remainingFrames;
        AmInt16 _oldFrames[6];
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_MIXER_H
