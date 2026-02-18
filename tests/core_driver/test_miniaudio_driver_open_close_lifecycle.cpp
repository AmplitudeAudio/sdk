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

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include "MiniAudioEngineTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(MiniAudioEngineTestCase, core_driver, miniaudio_driver_open_close_lifecycle)
    {
    public:
        void Run() override
        {
            auto driver = amEngine->GetDriver();
            AM_EXPECT(driver != nullptr);

            // Skip MiniAudio-specific tests if the engine fell back to null driver
            // (e.g., in CI environments without audio hardware)
            if (std::string(driver->GetName()) != "miniaudio")
                return;

            const auto& deviceDesc = driver->GetDeviceDescription();

            AM_EXPECT(deviceDesc.mDeviceState == eDeviceState_Started);
            AM_EXPECT(deviceDesc.mDeviceOutputSampleRate > 0);
            AM_EXPECT(deviceDesc.mDeviceOutputChannels != PlaybackOutputChannels::Default);
            AM_EXPECT(deviceDesc.mDeviceOutputFormat != PlaybackOutputFormat::Default);

            AM_EXPECT(Deinitialize());

            const auto& closedDeviceDesc = driver->GetDeviceDescription();
            AM_EXPECT(closedDeviceDesc.mDeviceState == eDeviceState_Closed);
        }
    };

    AM_REGISTER_TEST(core_driver, miniaudio_driver_open_close_lifecycle);
} // namespace SparkyStudios::Audio::Amplitude::Tests
