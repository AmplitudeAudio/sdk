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
    AM_TEST_CASE(EngineTestCase, core_engine, cannot_load_invalid_switch_handles)
    {
    public:
        void Run() override
        {
            // Test loading invalid switch handles
            SwitchHandle invalidSwitch = amEngine->GetSwitchHandle("invalid_switch");
            AM_EXPECT(invalidSwitch == nullptr);

            SwitchHandle invalidSwitch2 = amEngine->GetSwitchHandle(99999);
            AM_EXPECT(invalidSwitch2 == nullptr);

            // Test setting switch states with invalid switch IDs (should not crash)
            amEngine->SetSwitchState(99999, 1);
            amEngine->SetSwitchState(99999, "unknown");
            amEngine->SetSwitchState(99999, SwitchState());

            // Test setting switch states with invalid switch names (should not crash)
            amEngine->SetSwitchState("invalid_switch", 1);
            amEngine->SetSwitchState("invalid_switch", "unknown");
            amEngine->SetSwitchState("invalid_switch", SwitchState());

            // Test setting switch states with invalid switch handles (should not crash)
            amEngine->SetSwitchState(invalidSwitch, 1);
            amEngine->SetSwitchState(invalidSwitch2, "unknown");
            amEngine->SetSwitchState(invalidSwitch, SwitchState());
        }
    };

    AM_REGISTER_TEST(core_engine, cannot_load_invalid_switch_handles);
} // namespace SparkyStudios::Audio::Amplitude::Tests
