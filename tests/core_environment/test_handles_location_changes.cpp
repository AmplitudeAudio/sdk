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

#include "ComponentTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(ComponentTestCase, core_environment, handles_location_changes)
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
            constexpr AmVector3 location = { 10, 20, 30 };
            state.SetLocation(location);

            AM_EXPECT_EQ(state.GetLocation(), location);
            AM_EXPECT_EQ(zone->GetLocation(), location);

            state.Update();

            AM_EXPECT_EQ(state.GetLocation(), location);
            AM_EXPECT_EQ(zone->GetLocation(), location);

            // Test with wrapper
            Environment wrapper(&state);
            AM_EXPECT_EQ(wrapper.GetState(), &state);

            wrapper.SetZone(zone);
            AM_EXPECT_EQ(wrapper.GetZone(), zone);

            constexpr AmVector3 wrapperLocation = { 100, 200, 300 };
            wrapper.SetLocation(wrapperLocation);

            AM_EXPECT_EQ(wrapper.GetLocation(), wrapperLocation);
            AM_EXPECT_EQ(wrapper.GetLocation(), state.GetLocation());
            AM_EXPECT_EQ(state.GetLocation(), zone->GetLocation());

            wrapper.Update();

            AM_EXPECT_EQ(wrapper.GetLocation(), wrapperLocation);
            AM_EXPECT_EQ(wrapper.GetLocation(), state.GetLocation());
            AM_EXPECT_EQ(state.GetLocation(), zone->GetLocation());
        }
    };

    AM_REGISTER_TEST(core_environment, handles_location_changes);
} // namespace SparkyStudios::Audio::Amplitude::Tests
