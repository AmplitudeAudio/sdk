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

#ifndef _AM_CORE_DRIVER_H
#define _AM_CORE_DRIVER_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>
#include <SparkyStudios/Audio/Amplitude/Core/Device.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief Base class for audio device driver implementations.
     *
     * A driver allows to use an audio device to output sounds and
     * receive data from the microphone.
     *
     * @ingroup engine
     */
    class AM_API_PUBLIC Driver
    {
    public:
        /**
         * @brief Creates a new AudioDriver with an unique name.
         *
         * @param[in] name The driver name. Recommended names are "APIName".
         * eg. "MiniAudio" or "PortAudio" or "SDL", etc...
         */
        explicit Driver(AmString name);

        /**
         * @brief Default destructor.
         */
        virtual ~Driver();

        /**
         * @brief Open and start using the audio device.
         *
         * @param[in] device The audio device to use description to use for initializing the physical device.
         *
         * @return `true` if successful, `false` otherwise.
         */
        virtual bool Open(const DeviceDescription& device) = 0;

        /**
         * @brief Closes the audio device.
         *
         * @return `true` if successful, `false` otherwise.
         */
        virtual bool Close() = 0;

        /**
         * @brief Enumerates all the available audio devices.
         *
         * @param[out] devices The vector in which to store the device descriptions.
         *
         * @return `true` if successful, `false` otherwise.
         */
        virtual bool EnumerateDevices(std::vector<DeviceDescription>& devices) = 0;

        /**
         * @brief Gets the name of this driver.
         *
         * @return The name of this driver.
         */
        [[nodiscard]] const AmString& GetName() const;

        /**
         * @brief Gets the description of the device currently managed by this driver.
         *
         * @return The device description.
         */
        [[nodiscard]] const DeviceDescription& GetDeviceDescription() const;

        /**
         * @brief Registers a new audio driver.
         *
         * @param[in] driver The audio driver to add in the registry.
         */
        static void Register(Driver* driver);

        /**
         * @brief Unregisters an audio driver.
         *
         * @param[in] driver The audio driver to remove from the registry.
         */
        static void Unregister(const Driver* driver);

        /**
         * @brief Choose the most preferred audio driver.
         *
         * @return The default audio driver.
         */
        static Driver* Default();

        /**
         * @brief Look up a driver by name.
         *
         * @param[in] name The name of the audio driver. Must be registered before.
         *
         * @return The audio driver with the given name, or `nullptr` if none.
         */
        static Driver* Find(const AmString& name);

        /**
         * @brief Set the default diver to use in the engine.
         *
         * @param[in] name The name of the audio driver. Must be registered before.
         */
        static void SetDefault(const AmString& name);

        /**
         * @brief Locks the drivers registry.
         *
         * @warning This function is mainly used for internal purposes. It's
         * called before the `Engine` initialization, to discard the registration
         * of new divers after the engine is fully loaded.
         */
        static void LockRegistry();

        /**
         * @brief Unlocks the drivers registry.
         *
         * @warning This function is mainly used for internal purposes. It's
         * called after the `Engine` deinitialization, to allow the registration
         * of new divers after the engine is fully unloaded.
         */
        static void UnlockRegistry();

    protected:
        /**
         * @brief The driver name.
         */
        AmString m_name;

        /**
         * @brief The device description.
         */
        DeviceDescription m_deviceDescription;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_CORE_DRIVER_H
