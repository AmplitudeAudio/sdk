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
    AM_TEST_CASE(EngineTestCase, core_engine, switch_can_change_state_by_value)
    {
    public:
        void Run() override
        {
            SwitchHandle surfaceSwitch = amEngine->GetSwitchHandle("surface_type");
            AM_EXPECT(surfaceSwitch != nullptr);

            // Define switch states
            const SwitchState metal{ 3, "metal" };
            const SwitchState snow{ 5, "snow" };
            const SwitchState forest{ 1, "forest" };
            const SwitchState invalid{};

            // Test setting switch state by value
            surfaceSwitch->SetState(metal);
            AM_EXPECT(surfaceSwitch->GetState() == metal);

            // Test setting invalid state for this switch (should keep previous state)
            surfaceSwitch->SetState(forest);
            AM_EXPECT(surfaceSwitch->GetState() != forest);
            AM_EXPECT(surfaceSwitch->GetState() == metal);

            // Test setting invalid state (should keep previous state)
            surfaceSwitch->SetState(invalid);
            AM_EXPECT(surfaceSwitch->GetState() != invalid);
            AM_EXPECT(surfaceSwitch->GetState() == metal);

            // Test engine can change state by handle and value
            amEngine->SetSwitchState(surfaceSwitch, snow);
            AM_EXPECT(surfaceSwitch->GetState() == snow);

            // Test engine can change state by ID and value
            amEngine->SetSwitchState(surfaceSwitch->GetId(), snow);
            AM_EXPECT(surfaceSwitch->GetState() == snow);

            // Test engine can change state by name and value
            amEngine->SetSwitchState(surfaceSwitch->GetName(), snow);
            AM_EXPECT(surfaceSwitch->GetState() == snow);
        }
    };

    AM_REGISTER_TEST(core_engine, switch_can_change_state_by_value);
} // namespace SparkyStudios::Audio::Amplitude::Tests
