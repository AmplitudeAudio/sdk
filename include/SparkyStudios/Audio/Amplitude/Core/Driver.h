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

#ifndef SS_AMPLITUDE_AUDIO_DRIVER_H
#define SS_AMPLITUDE_AUDIO_DRIVER_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

namespace SparkyStudios::Audio::Amplitude
{
    struct EngineConfigDefinition;

    class Mixer;

    /**
     * @brief Base class for audio device driver implementations.
     *
     * A driver allows to use an audio device to output sounds and
     * receive data from the microphone.
     */
    class Driver
    {
    public:
        /**
         * @brief Creates a new AudioDriver with an unique name.
         *
         * @param name The driver name. Recommended names are "APIName".
         * eg. "MiniAudio" or "PortAudio".
         */
        explicit Driver(AmString name);

        virtual ~Driver() = default;

        /**
         * @brief Initializes the audio driver.
         *
         * @param mixer The audio mixer object backed by the engine.
         */
        void Initialize(Mixer* mixer)
        {
            m_mixer = mixer;
        }

        /**
         * @brief Open and start using the audio device.
         *
         * @param config The audio engine configuration used when initializing Amplitude.
         */
        virtual bool Open(const EngineConfigDefinition* config) = 0;

        /**
         * @brief CLoses the audio device.
         */
        virtual bool Close() = 0;

        /**
         * @brief Gets the name of this driver.
         *
         * @return The name of this driver.
         */
        [[nodiscard]] AmString GetName() const
        {
            return m_name;
        }

        /**
         * @brief Registers a new audio driver.
         *
         * @param driver The audio driver to add in the registry.
         */
        static void Register(Driver* driver);

        /**
         * @brief Choose the most preferred audio driver.
         *
         * @return The default audio driver.
         */
        static Driver* Default();

        /**
         * @brief Look up a driver by name.
         *
         * @return The audio driver with the given name, or NULL if none.
         */
        static Driver* Find(AmString name);

        /**
         * @brief Set the default diver to use in the engine.
         *
         * @param name The name of the audio driver. Must be registered before.
         */
        static void SetDefault(AmString name);

    protected:
        /**
         * @brief The driver name.
         */
        AmString m_name;

        /**
         * @brief The mixer instance provided by the engine.
         */
        Mixer* m_mixer;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_DRIVER_H
