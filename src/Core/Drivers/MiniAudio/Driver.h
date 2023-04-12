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

#ifndef SS_AMPLITUDE_AUDIO_MINIAUDIO_DRIVER_H
#define SS_AMPLITUDE_AUDIO_MINIAUDIO_DRIVER_H

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Utils/miniaudio/miniaudio_utils.h>

namespace SparkyStudios::Audio::Amplitude::Drivers
{
    [[maybe_unused]] static class MiniAudioDriver final : public Driver
    {
    public:
        MiniAudioDriver()
            : Driver("miniaudio")
            , _initialized(false)
            , _device()
        {}

        ~MiniAudioDriver() final;

        bool Open(const DeviceDescription& device) final;

        bool Close() final;

    protected:
        friend void miniaudio_device_notification(const ma_device_notification* pNotification);

    private:
        bool _initialized;
        ma_device _device;
    } miniaudio_driver; // NOLINT(cert-err58-cpp)
} // namespace SparkyStudios::Audio::Amplitude::Drivers

#endif // SS_AMPLITUDE_AUDIO_MINIAUDIO_DRIVER_H
