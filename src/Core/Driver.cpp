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

#include <cstring>
#include <map>

#include <SparkyStudios/Audio/Amplitude/Core/Driver.h>

namespace SparkyStudios::Audio::Amplitude
{
    typedef std::map<AmString, Driver*> DriverRegistry;
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

    Driver::Driver(AmString name)
        : m_name(name)
        , m_mixer(nullptr)
    {
        Driver::Register(this);
    }

    void Driver::Initialize(Mixer* mixer)
    {
        m_mixer = mixer;
    }

    AmString Driver::GetName() const
    {
        return m_name;
    }

    void Driver::Register(Driver* driver)
    {
        if (lockDrivers())
            return;

        if (!Find(driver->GetName()))
        {
            DriverRegistry& drivers = driverRegistry();
            drivers.insert(DriverImpl(driver->GetName(), driver));
            driversCount()++;
        }
    }

    Driver* Driver::Default()
    {
        DriverRegistry& drivers = driverRegistry();
        if (!drivers.empty())
            return drivers.rbegin()->second;
        return nullptr;
    }

    Driver* Driver::Find(AmString name)
    {
        DriverRegistry& drivers = driverRegistry();
        for (auto&& driver : drivers)
        {
            if (strcmp(driver.second->m_name, name) == 0)
                return driver.second;
        }
        return nullptr;
    }

    void Driver::SetDefault(AmString name)
    {
        DriverRegistry& drivers = driverRegistry();
        for (auto i = drivers.cbegin(), e = drivers.cend(); i != e; ++i)
        {
            if (strcmp(i->second->m_name, name) == 0)
            {
                std::pair<AmString, Driver*> node = DriverImpl(i->first, i->second);
                drivers.erase(i);
                drivers.insert(node);
                return;
            }
        }
    }

    void Driver::LockRegistry()
    {
        lockDrivers() = true;
    }
} // namespace SparkyStudios::Audio::Amplitude