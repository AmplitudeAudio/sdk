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

#include "EngineTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(EngineTestCase, core_driver, driver_has_valid_device_description)
    {
    public:
        void Run() override
        {
            auto driver = amEngine->GetDriver();
            AM_EXPECT(driver != nullptr);

            const auto& desc = driver->GetDeviceDescription();

            AM_EXPECT(desc.mRequestedOutputSampleRate > 0);
            AM_EXPECT(desc.mOutputBufferSize > 0);
            AM_EXPECT(desc.mRequestedOutputChannels != PlaybackOutputChannels::Default);
            AM_EXPECT(desc.mRequestedOutputFormat != PlaybackOutputFormat::Default);
        }
    };

    AM_REGISTER_TEST(core_driver, driver_has_valid_device_description);
} // namespace SparkyStudios::Audio::Amplitude::Tests
