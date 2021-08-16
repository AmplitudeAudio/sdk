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

#ifndef SPARK_AUDIO_REAL_CHANNEL_H
#define SPARK_AUDIO_REAL_CHANNEL_H

#include <SparkyStudios/Audio/Amplitude/Core/Fader.h>
#include <SparkyStudios/Audio/Amplitude/Math/HandmadeMath.h>

#include "atomix.h"

namespace SparkyStudios::Audio::Amplitude
{
    class EngineInternalState;

    class SoundCollection;
    class Sound;

    class Mixer;

    /**
     * @brief A RealChannel represents a channel of audio on the mixer.
     *
     * Not all channels are backed by RealChannels. If there are more
     *  channels of audio being played simultaneously than the mixer can handle,
     * the lowest priority channels will be virtualized. That is, they will no
     * longer have their audio mixed, but their GetGain value and position (and a few
     * other properties) will continue to be tracked.
     *
     * This class represents the real channel interface to the underlying audio
     * mixer backend being used.
     */
    class RealChannel
    {
        friend class Mixer;

    public:
        RealChannel();

        /**
         * @brief Initialize this channel.
         */
        void Initialize(int index);

        /**
         * @brief Play the audio on the real channel.
         */
        bool Play(SoundCollection* handle, Sound* sound);

        /**
         * @brief Halt the real channel so it may be re-used. However this virtual channel may still be considered playing.
         */
        void Halt();

        /**
         * @brief Pause the real channel.
         */
        void Pause();

        /**
         * @brief Resume the paused real channel.
         */
        void Resume();

        /**
         * @brief Check if this channel is currently playing on a real channel.
         */
        [[nodiscard]] bool Playing() const;

        /**
         * @brief Check if this channel is currently paused on a real channel.
         */
        [[nodiscard]] bool Paused() const;

        /**
         * @brief Set the current GetGain of the real channel.
         */
        void SetGain(float gain);

        /**
         * @brief Get the current GetGain of the real channel.
         */
        [[nodiscard]] float GetGain() const;

        /**
         * @brief Set the pan for the sound. This should be a unit vector.
         */
        void SetPan(const hmm_vec2& pan);

        /**
         * @brief Fade this channel out over the given number of milliseconds.
         */
        void FadeOut(int milliseconds);

        /**
         * @brief Return true if this is a valid real channel.
         */
        [[nodiscard]] bool Valid() const;

    private:
        bool _activeFader;

        float _dopplerValue;

        // Sample rate; sampleRate = audioEngineOutputFrequency * relativePlaySpeed
        float _sampleRate;

        // Set volume
        float _setVolume;
        // 3D volume
        float _3DVolume;
        // Overall volume; overallVolume = setVolume * 3DVolume
        float _overallVolume;
        // Current channel volumes, used to ramp the volume changes to avoid clicks
        float _currentChannelVolume[MAX_CHANNELS]{};
        // Volume for each channel (panning)
        float _channelVolume[MAX_CHANNELS]{};

        // Relative play speed
        float _setRelativePlaySpeed;
        // Overall relative play speed; overall = dopplerValue * setRelativePlaySpeed
        float _overallRelativePlaySpeed;
        // How long this stream has played, in seconds.
        AmTime _streamTime;
        // Position of this stream, in seconds.
        AmTime _streamPosition;

        // Fader for the audio panning
        Fader _panFader;
        // Fader for the audio volume
        Fader _volumeFader;
        // Fader for the relative Play speed
        Fader _relativePlaySpeedFader;
        // Fader used to schedule pausing of the stream
        Fader _pauseScheduler;
        // Fader used to schedule stopping of the stream
        Fader _stopScheduler;

        AmHandle _channelId;
        bool _stream;
        bool _loop;

        float _pan;
        float _gain;

        atomix_mixer* _mixer;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SPARK_AUDIO_REAL_CHANNEL_H
