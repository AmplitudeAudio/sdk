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

#include <map>

#include <SparkyStudios/Audio/Amplitude/Core/Driver.h>
#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>
#include <SparkyStudios/Audio/Amplitude/IO/Log.h>

namespace SparkyStudios::Audio::Amplitude
{
    typedef std::map<AmString, std::shared_ptr<Driver>> DriverRegistry;
    typedef DriverRegistry::value_type DriverImpl;

    static DriverRegistry& driverRegistry()
    {
        static DriverRegistry r;
        return r;
    }

    static bool& lockDrivers()
    {
        static bool b = false;
        return b;
    }

    static AmUInt32& driversCount()
    {
        static AmUInt32 c = 0;
        return c;
    }

    static AmString& defaultDriverName()
    {
        static AmString name;
        return name;
    }

    Driver::Driver(AmString name)
        : m_name(std::move(name))
        , m_deviceDescription()
    {}

    Driver::~Driver()
    {}

    const AmString& Driver::GetName() const
    {
        return m_name;
    }

    const DeviceDescription& Driver::GetDeviceDescription() const
    {
        return m_deviceDescription;
    }

    void Driver::Register(std::shared_ptr<Driver> driver)
    {
        if (lockDrivers() || driver == nullptr)
            return;

        if (Find(driver->GetName()) != nullptr)
        {
            amLogWarning("Failed to register driver '%s' as it is already registered.", driver->GetName().c_str());
            return;
        }

        DriverRegistry& drivers = driverRegistry();
        drivers.insert(DriverImpl(driver->GetName(), driver));
        driversCount()++;
    }

    void Driver::Unregister(std::shared_ptr<const Driver> driver)
    {
        if (lockDrivers() || driver == nullptr)
            return;

        DriverRegistry& drivers = driverRegistry();
        if (const auto& it = drivers.find(driver->GetName()); it != drivers.end())
        {
            drivers.erase(it);
            driversCount()--;
        }
    }

    std::shared_ptr<Driver> Driver::Default()
    {
        const DriverRegistry& drivers = driverRegistry();

        if (drivers.empty())
            return nullptr;

        const AmString& defaultName = defaultDriverName();
        if (!defaultName.empty())
            if (const auto& it = drivers.find(defaultName); it != drivers.end())
                return it->second;

        return drivers.rbegin()->second;
    }

    std::shared_ptr<Driver> Driver::Find(const AmString& name)
    {
        const DriverRegistry& drivers = driverRegistry();
        if (const auto& it = drivers.find(name); it != drivers.end())
            return it->second;

        return nullptr;
    }

    void Driver::SetDefault(const AmString& name)
    {
        DriverRegistry& drivers = driverRegistry();
        if (const auto& it = drivers.find(name); it != drivers.end())
            defaultDriverName() = name;
    }

    void Driver::ResetDefault()
    {
        defaultDriverName().clear();
    }

    void Driver::LockRegistry()
    {
        lockDrivers() = true;
    }

    void Driver::UnlockRegistry()
    {
        lockDrivers() = false;
    }

    const std::map<AmString, std::shared_ptr<Driver>>& Driver::GetRegistry()
    {
        return driverRegistry();
    }
} // namespace SparkyStudios::Audio::Amplitude
