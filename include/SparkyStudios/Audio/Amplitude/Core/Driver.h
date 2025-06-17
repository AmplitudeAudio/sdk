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
     * @brief Base class used to create device drivers.
     *
     * A driver allows using an audio device to output sounds on speakers and
     * receive data from the microphone.
     *
     * @ingroup engine
     */
    class AM_API_PUBLIC Driver
    {
    public:
        /**
         * @brief Creates a new driver with a unique name.
         *
         * @param[in] name The driver's name. Recommended names are "APIName", e.g., "MiniAudio" or "PortAudio" or "SDL", etc...
         */
        explicit Driver(AmString name);

        /**
         * @brief Default destructor.
         */
        virtual ~Driver();

        /**
         * @brief Open and start using the audio device.
         *
         * @param[in] device The audio device description to use for initializing the physical device.
         *
         * @return @c true if successful, @c false otherwise.
         */
        virtual bool Open(const DeviceDescription& device) = 0;

        /**
         * @brief Closes the audio device.
         *
         * @return @c true if successful, @c false otherwise.
         */
        virtual bool Close() = 0;

        /**
         * @brief Enumerates all the available audio devices.
         *
         * @param[out] devices The vector in which to store the device descriptions.
         *
         * @return @c true if successful, @c false otherwise.
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
         * @note This method does nothing if the registry is locked.
         *
         * @param[in] driver The audio driver to add in the registry.
         *
         * @see LockRegistry, UnlockRegistry
         */
        static void Register(std::shared_ptr<Driver> driver);

        /**
         * @brief Unregisters an audio driver.
         *
         * @note This method does nothing if the registry is locked.
         *
         * @param[in] driver The audio driver to remove from the registry.
         *
         * @see LockRegistry, UnlockRegistry
         */
        static void Unregister(std::shared_ptr<const Driver> driver);

        /**
         * @brief Choose the most preferred audio driver.
         *
         * This method will return the driver instance corresponding to the name provided in the loaded
         * engine configuration. [Read this page](/project/engine-config/#driver) to learn more.
         *
         * @return The default audio driver.
         */
        static std::shared_ptr<Driver> Default();

        /**
         * @brief Look up a driver by name.
         *
         * @param[in] name The name of the audio driver. Must be registered before.
         *
         * @return The audio driver with the given name, or @c nullptr if not found.
         */
        static std::shared_ptr<Driver> Find(const AmString& name);

        /**
         * @brief Set the default diver to use in the engine.
         *
         * This method will overwrite the default driver set from the loaded engine configuration.
         *
         * @param[in] name The name of the audio driver. Must be registered before.
         */
        static void SetDefault(const AmString& name);

        /**
         * @brief Locks the drivers' registry.
         *
         * @warning This function is mainly used for internal purposes. It's
         * called before the @c Engine initialization, to discard the registration
         * of new divers after the engine is fully loaded.
         */
        static void LockRegistry();

        /**
         * @brief Unlocks the drivers' registry.
         *
         * @warning This function is mainly used for internal purposes. It's
         * called after the @c Engine deinitialization, to allow the registration
         * of new divers after the engine is fully unloaded.
         */
        static void UnlockRegistry();

        /**
         * @brief Gets the list of registered Drivers.
         *
         * @return The registry of Drivers.
         */
        static const std::map<AmString, std::shared_ptr<Driver>>& GetRegistry();

    protected:
        /**
         * @brief The driver's name.
         */
        AmString m_name;

        /**
         * @brief The device description.
         */
        DeviceDescription m_deviceDescription;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_CORE_DRIVER_H
