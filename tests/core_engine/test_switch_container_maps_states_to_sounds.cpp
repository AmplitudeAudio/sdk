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
    AM_TEST_CASE(EngineTestCase, core_engine, switch_container_maps_states_to_sounds)
    {
    public:
        void Run() override
        {
            SwitchContainerHandle container = amEngine->GetSwitchContainerHandle("footsteps");
            AM_EXPECT_NOT(container == nullptr);

            // Verify the switch reference is correct (surface_type, ID=1)
            const Switch* sw = container->GetSwitch();
            AM_EXPECT_NOT(sw == nullptr);
            AM_EXPECT_EQ(sw->GetId(), static_cast<AmSwitchID>(1));

            // Verify container properties
            AM_EXPECT(container->GetSpatialization() == eSpatialization_Position);
            AM_EXPECT(container->GetScope() == eScope_Entity);

            // State 4 (grass) should map to collection 1003
            {
                const auto& items = container->GetSoundObjects(4);
                AM_EXPECT(items.size() == 1);
                AM_EXPECT_EQ(items[0].m_id, static_cast<AmObjectID>(1003));
            }

            // State 5 (snow) should map to collection 1004
            {
                const auto& items = container->GetSoundObjects(5);
                AM_EXPECT(items.size() == 1);
                AM_EXPECT_EQ(items[0].m_id, static_cast<AmObjectID>(1004));
            }

            // State 3 (metal) should map to collection 1005
            {
                const auto& items = container->GetSoundObjects(3);
                AM_EXPECT(items.size() == 1);
                AM_EXPECT_EQ(items[0].m_id, static_cast<AmObjectID>(1005));
            }

            // Faders should be valid for each entry
            AM_EXPECT_NOT(container->GetFaderIn(1003) == nullptr);
            AM_EXPECT_NOT(container->GetFaderOut(1003) == nullptr);
        }
    };

    AM_REGISTER_TEST(core_engine, switch_container_maps_states_to_sounds);
} // namespace SparkyStudios::Audio::Amplitude::Tests
