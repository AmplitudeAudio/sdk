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

#include <Core/Drivers/Null/Driver.h>

#include <Mixer/Amplimix.h>
#include <Utils/ScopedDenormalFlush.h>

namespace SparkyStudios::Audio::Amplitude
{
    static void null_mix(void* param)
    {
        ScopedDenormalFlush denormalFlush;

        const auto* data = static_cast<NullDriver*>(param);

        while (data->IsRunning())
        {
            if (Engine::GetInstance()->IsStopping())
                break;

            Engine::GetInstance()->GetMixer()->Mix(nullptr, data->GetDeviceDescription().mOutputBufferSize);
            Thread::Sleep(10);
        }
    }

    NullDriver::NullDriver()
        : Driver("null")
        , _initialized(false)
        , _running(false)
        , _thread(nullptr)
    {}

    NullDriver::~NullDriver()
    {
        Close();
    }

    bool NullDriver::Open(const DeviceDescription& device)
    {
        if (_initialized)
            return true;

        m_deviceDescription = device;

        CallDeviceNotificationCallback(eDeviceNotification_Opened, device, this);
        m_deviceDescription.mDeviceState = eDeviceState_Opened;

        _running = true;

        _thread = Thread::CreateThread(null_mix, this);

        _initialized = true;

        CallDeviceNotificationCallback(eDeviceNotification_Started, device, this);
        m_deviceDescription.mDeviceState = eDeviceState_Started;

        return true;
    }

    bool NullDriver::Close()
    {
        if (_initialized)
        {
            _running = false;
            CallDeviceNotificationCallback(eDeviceNotification_Stopped, m_deviceDescription, this);

            Thread::Wait(_thread);

            m_deviceDescription.mOutputBufferSize = 0;

            _initialized = false;
            CallDeviceNotificationCallback(eDeviceNotification_Closed, m_deviceDescription, this);
        }

        return true;
    }

    bool NullDriver::EnumerateDevices(std::vector<DeviceDescription>& devices)
    {
        return true;
    }
} // namespace SparkyStudios::Audio::Amplitude
