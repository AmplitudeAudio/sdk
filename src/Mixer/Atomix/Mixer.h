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

#ifndef SPARK_AUDIO_MIXER_H
#define SPARK_AUDIO_MIXER_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

#include <SparkyStudios/Audio/Amplitude/Core/Fader.h>

#include <Core/BusInternalState.h>
#include <Core/Drivers/MiniAudio/miniaudio.h>
#include <Utils/intrusive_list.h>

#include "audio_config_definition_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
    struct EngineConfigDefinition;

    /**
     * @brief This class represents the audio mixer backend that does the actual audio mixing.
     *
     * This class represents the mixer interface to the underlying audio mixer
     * backend being used.
     */
    class Mixer
    {
    public:
        explicit Mixer(EngineInternalState* parent);
        ~Mixer();

        /**
         * @brief Initializes the audio Mixer.
         *
         * @param config The audio engine configuration.
         * @return true on success, false on failure.
         */
        bool Initialize(const EngineConfigDefinition* config);

        /**
         * @brief Process the audio data and output the result.
         *
         * @param mixBuffer The buffer in which output the result.
         * @param frameCount The number of audio frames to process.
         */
        void Mix(AmVoidPtr mixBuffer, AmUInt32 frameCount);

        /**
         * @brief Returns the user data associated to this Sound.
         * @return The user data.
         */
        [[nodiscard]] AmVoidPtr GetUserData() const
        {
            return m_userData;
        }

        /**
         * @brief Sets the user data associated to this Sound.
         * @param userData The user data.
         */
        void SetUserData(AmVoidPtr userData)
        {
            m_userData = userData;
        }

        /**
         * @brief Mixer post initialization.
         *
         * This method is called once, just after the playback device is initialized
         * and before it is started.
         *
         * @param bufferSize The buffer size accepted by the playback device.
         */
        void PostInit(AmUInt32 bufferSize, AmUInt32 sampleRate, AmUInt32 channels);

    protected:
        AmVoidPtr m_userData;

    private:
        void discardUserData();

        void lockAudioMutex();
        void unlockAudioMutex();

        bool _initialized;

        AmVoidPtr _audioThreadMutex;
        bool _insideAudioThreadMutex;

        EngineInternalState* _engineState;
        const EngineConfigDefinition* _audioEngineConfig;
    };

    void miniaudio_mixer(ma_device* pDevice, AmVoidPtr pOutput, const void* pInput, ma_uint32 frameCount);
    void miniaudio_uninit(Mixer* mixer);
    bool miniaudio_init(Mixer* mixer, const EngineConfigDefinition* audioEngineConfig, ma_format format);
} // namespace SparkyStudios::Audio::Amplitude

#endif // SPARK_AUDIO_MIXER_H
