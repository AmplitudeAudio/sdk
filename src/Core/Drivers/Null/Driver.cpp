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

#include <chrono>
#include <thread>

#include <Core/Drivers/Null/Driver.h>

#include <Mixer/Amplimix.h>
#include <Utils/ScopedDenormalFlush.h>

namespace SparkyStudios::Audio::Amplitude
{
    static void null_mix(void* param)
    {
        ScopedDenormalFlush denormalFlush;

        const auto* data = static_cast<NullDriver*>(param);

        const auto channelsCount = static_cast<AmInt16>(data->GetDeviceDescription().mRequestedOutputChannels);

        // Cache engine access outside the loop: Engine::GetInstance() locks a mutex.
        auto* engine = Engine::GetInstance();
        auto* mixer = engine != nullptr ? engine->GetMixer() : nullptr;

        using Clock = std::chrono::steady_clock;

        const auto& description = data->GetDeviceDescription();
        const AmUInt64 frames = channelsCount > 0 ? description.mOutputBufferSize / channelsCount : 0;
        const AmUInt32 sampleRate = description.mRequestedOutputSampleRate;

        const auto period = std::chrono::duration_cast<Clock::duration>(
            std::chrono::duration<double>(sampleRate > 0 ? static_cast<double>(frames) / sampleRate : 0.01));

        constexpr auto maxLag = std::chrono::milliseconds(100);

        auto deadline = Clock::now();

        while (data->IsRunning())
        {
            if (engine == nullptr || engine->IsStopping())
                break;

            if (mixer != nullptr)
                mixer->Mix(nullptr, static_cast<AmUInt64>(frames));

            deadline += period;

            const auto now = Clock::now();
            if (now > deadline + maxLag)
                deadline = now;
            else if (now < deadline)
                std::this_thread::sleep_until(deadline);
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
