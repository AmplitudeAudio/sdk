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
    AM_TEST_CASE(EngineTestCase, core_engine, switch_can_change_state_by_id)
    {
    public:
        void Run() override
        {
            SwitchHandle envSwitch = amEngine->GetSwitchHandle("env");
            AM_EXPECT(envSwitch != nullptr);

            // Test setting switch state by ID
            envSwitch->SetState(22);
            AM_EXPECT(envSwitch->GetState().m_name == "desert");

            // Test setting invalid ID (should keep previous state)
            envSwitch->SetState(25);
            AM_EXPECT(envSwitch->GetState().m_name != "snow");
            AM_EXPECT(envSwitch->GetState().m_name == "desert");

            // Test engine can change state by handle and ID
            amEngine->SetSwitchState(envSwitch, 21);
            AM_EXPECT(envSwitch->GetState().m_name == "forest");

            // Test engine can change state by ID and ID
            amEngine->SetSwitchState(envSwitch->GetId(), 21);
            AM_EXPECT(envSwitch->GetState().m_name == "forest");

            // Test engine can change state by name and ID
            amEngine->SetSwitchState(envSwitch->GetName(), 21);
            AM_EXPECT(envSwitch->GetState().m_name == "forest");
        }
    };

    AM_REGISTER_TEST(core_engine, switch_can_change_state_by_id);
} // namespace SparkyStudios::Audio::Amplitude::Tests
