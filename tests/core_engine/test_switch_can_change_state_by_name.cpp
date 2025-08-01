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

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    void EngineTestCase::Run()
    {
        SwitchHandle envSwitch = amEngine->GetSwitchHandle("env");
        AM_EXPECT(envSwitch != nullptr);

        // Test setting switch state by name
        envSwitch->SetState("desert");
        AM_EXPECT(envSwitch->GetState().m_id == 2);

        // Test setting invalid name (should keep previous state)
        envSwitch->SetState("metal");
        AM_EXPECT(envSwitch->GetState().m_id != 3);
        AM_EXPECT(envSwitch->GetState().m_id == 2);

        // Test engine can change state by handle and name
        amEngine->SetSwitchState(envSwitch, "forest");
        AM_EXPECT(envSwitch->GetState().m_id == 1);

        // Test engine can change state by ID and name
        amEngine->SetSwitchState(envSwitch->GetId(), "forest");
        AM_EXPECT(envSwitch->GetState().m_id == 1);

        // Test engine can change state by name and name
        amEngine->SetSwitchState(envSwitch->GetName(), "forest");
        AM_EXPECT(envSwitch->GetState().m_id == 1);
    }
} // namespace SparkyStudios::Audio::Amplitude::Tests
