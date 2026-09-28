// Copyright (c) 2026-present Sparky Studios. All rights reserved.
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

#include <Fidelity/OfflineDriver.h>

namespace SparkyStudios::Audio::Amplitude::Fidelity
{
    OfflineDriver::OfflineDriver()
        : Driver("offline")
    {}

    OfflineDriver::~OfflineDriver()
    {
        Close();
    }

    bool OfflineDriver::Open(const DeviceDescription& device)
    {
        if (_open)
            return true;

        m_deviceDescription = device;

        CallDeviceNotificationCallback(eDeviceNotification_Opened, device, this);
        m_deviceDescription.mDeviceState = eDeviceState_Opened;

        _open = true;

        CallDeviceNotificationCallback(eDeviceNotification_Started, device, this);
        m_deviceDescription.mDeviceState = eDeviceState_Started;

        return true;
    }

    bool OfflineDriver::Close()
    {
        if (!_open)
            return true;

        CallDeviceNotificationCallback(eDeviceNotification_Stopped, m_deviceDescription, this);
        _open = false;
        CallDeviceNotificationCallback(eDeviceNotification_Closed, m_deviceDescription, this);

        return true;
    }

    bool OfflineDriver::EnumerateDevices(std::vector<DeviceDescription>& devices)
    {
        AM_UNUSED(devices);
        return true;
    }

    bool OfflineDriver::IsOpen() const
    {
        return _open;
    }
} // namespace SparkyStudios::Audio::Amplitude::Fidelity
