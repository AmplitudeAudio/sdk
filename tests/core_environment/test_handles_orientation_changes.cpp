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

#include <Core/EnvironmentInternalState.h>

#include "SimpleTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(SimpleTestCase, core_environment, handles_orientation_changes)
    {
    public:
        void Run() override
        {
            EnvironmentInternalState state;
            state.SetId(1);

            fplutil::intrusive_list environment_list(&EnvironmentInternalState::node);
            environment_list.push_back(state);

            auto inner = std::make_shared<SphereShape>(10);
            auto outer = std::make_shared<SphereShape>(20);
            auto zone = std::make_shared<SphereZone>(inner, outer);

            state.SetZone(zone);
            AM_EXPECT_EQ(state.GetZone(), zone);

            // Test state directly
            constexpr AmVector3 direction = kVector3UnitX;
            constexpr AmVector3 up = kVector3UnitZ;

            const auto orientation = Orientation(direction, up);
            state.SetOrientation(orientation);

            AM_EXPECT_EQ(state.GetDirection(), direction);
            AM_EXPECT_EQ(state.GetUp(), up);

            AM_EXPECT_EQ(zone->GetDirection(), direction);
            AM_EXPECT_EQ(zone->GetUp(), up);

            AM_EXPECT(std::memcmp(&state.GetOrientation(), &orientation, sizeof(Orientation)) == 0);
            AM_EXPECT(std::memcmp(&zone->GetOrientation(), &orientation, sizeof(Orientation)) == 0);

            // Test with wrapper
            Environment wrapper(&state);
            AM_EXPECT_EQ(wrapper.GetState(), &state);

            constexpr AmVector3 wrapperDirection = { 1, 0, 0 };
            constexpr AmVector3 wrapperUp = { 0, 0, 1 };
            wrapper.SetOrientation(Orientation(wrapperDirection, wrapperUp));

            AM_EXPECT_EQ(wrapper.GetDirection(), wrapperDirection);
            AM_EXPECT_EQ(wrapper.GetUp(), wrapperUp);

            AM_EXPECT_EQ(wrapper.GetDirection(), state.GetDirection());
            AM_EXPECT_EQ(wrapper.GetUp(), state.GetUp());

            AM_EXPECT_EQ(state.GetDirection(), zone->GetDirection());
            AM_EXPECT_EQ(state.GetUp(), zone->GetUp());
        }
    };

    AM_REGISTER_TEST(core_environment, handles_orientation_changes);
} // namespace SparkyStudios::Audio::Amplitude::Tests
