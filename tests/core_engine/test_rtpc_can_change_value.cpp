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

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include "EngineTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(EngineTestCase, core_engine, rtpc_can_change_value)
    {
    public:
        void Run() override
        {
            RtpcHandle rtpc1 = amEngine->GetRtpcHandle(1);
            AM_EXPECT(rtpc1 != nullptr);

            RtpcHandle rtpc2 = amEngine->GetRtpcHandle("wind_force");
            AM_EXPECT(rtpc2 != nullptr);

            // Test direct RTPC value change
            rtpc1->SetValue(50);
            AM_EXPECT(rtpc1->GetValue() == 50.0);

            // Test RTPC with transition time
            rtpc2->SetValue(1000);
            AM_EXPECT_NOT(rtpc2->GetValue() == 1000);
            amEngine->WaitUntilFrames(65);
            AM_EXPECT(std::abs(rtpc2->GetValue() - 1000.0) < kEpsilon);

            // Test engine can change value by ID
            amEngine->SetRtpcValue(rtpc1->GetId(), 75);
            AM_EXPECT(rtpc1->GetValue() == 75.0);

            amEngine->SetRtpcValue(rtpc2->GetId(), 75);
            AM_EXPECT_NOT(rtpc2->GetValue() == 75.0);
            amEngine->WaitUntilFrames(35);
            AM_EXPECT(std::abs(rtpc2->GetValue() - 75.0) < kEpsilon);

            // Test engine can change value by name
            amEngine->SetRtpcValue(rtpc1->GetName(), 80);
            AM_EXPECT(rtpc1->GetValue() == 80.0);

            amEngine->SetRtpcValue(rtpc2->GetName(), 75000);
            AM_EXPECT_NOT(rtpc2->GetValue() == 75000.0);
            amEngine->WaitUntilFrames(65);
            AM_EXPECT(std::abs(rtpc2->GetValue() - 75000.0) < kEpsilon);

            // Test engine can change value by handle
            amEngine->SetRtpcValue(rtpc1, 90);
            AM_EXPECT(rtpc1->GetValue() == 90.0);

            amEngine->SetRtpcValue(rtpc2, 90);
            AM_EXPECT_NOT(rtpc2->GetValue() == 90.0);
            amEngine->WaitUntilFrames(35);
            AM_EXPECT(std::abs(rtpc2->GetValue() - 90.0) < kEpsilon);
        }
    };

    AM_REGISTER_TEST(core_engine, rtpc_can_change_value);
} // namespace SparkyStudios::Audio::Amplitude::Tests
