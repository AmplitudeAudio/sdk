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

#ifndef SS_AMPLITUDE_AUDIO_NULL_DRIVER_H
#define SS_AMPLITUDE_AUDIO_NULL_DRIVER_H

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

namespace SparkyStudios::Audio::Amplitude::Drivers
{
    struct NullDriverDeviceData
    {
        AmUInt32 mOutputBufferSize;
        AmVoidPtr mOutputBuffer;
        bool mRunning;
    };

    [[maybe_unused]] static class NullDriver final : public Driver
    {
    public:
        NullDriver();

        ~NullDriver() override;

        bool Open(const DeviceDescription& device) override;

        bool Close() override;

        bool EnumerateDevices(std::vector<DeviceDescription>& devices) override;

    private:
        bool _initialized;

        AmThreadHandle _thread;

        NullDriverDeviceData _deviceData;
    } g_driver_null; // NOLINT(cert-err58-cpp)
} // namespace SparkyStudios::Audio::Amplitude::Drivers

#endif // SS_AMPLITUDE_AUDIO_NULL_DRIVER_H
