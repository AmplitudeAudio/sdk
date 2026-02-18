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
    AM_TEST_CASE(MiniAudioEngineTestCase, core_driver, miniaudio_driver_can_be_initialized)
    {
    public:
        void Run() override
        {
            AM_EXPECT(amEngine->GetDriver() != nullptr);

            // Skip MiniAudio-specific tests if the engine fell back to null driver
            // (e.g., in CI environments without audio hardware)
            if (std::string(amEngine->GetDriver()->GetName()) != "miniaudio")
                return;

            const auto& deviceDesc = amEngine->GetDriver()->GetDeviceDescription();
            AM_EXPECT(deviceDesc.mDeviceState == eDeviceState_Started);

            AM_EXPECT(deviceDesc.mDeviceOutputSampleRate > 0);
            AM_EXPECT(deviceDesc.mDeviceOutputChannels != PlaybackOutputChannels::Default);
            AM_EXPECT(deviceDesc.mDeviceOutputFormat != PlaybackOutputFormat::Default);

            AM_EXPECT(Deinitialize());
        }
    };

    AM_REGISTER_TEST(core_driver, miniaudio_driver_can_be_initialized);
} // namespace SparkyStudios::Audio::Amplitude::Tests
