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

#include <Core/EntityInternalState.h>

#include "SimpleTestCase.h"

using namespace SparkyStudios::Audio::Amplitude;

void SimpleTestCase::Run()
{
    EntityInternalState state;
    state.SetId(1);

    fplutil::intrusive_list entity_list(&EntityInternalState::node);
    entity_list.push_back(state);

    // Test state directly
    constexpr AmEnvironmentID environment = 1;
    constexpr AmReal32 factor = 0.56f;
    state.SetEnvironmentFactor(environment, factor);

    AM_EXPECT_EQ(state.GetEnvironmentFactor(environment), factor);

    // Test unregistered environment returns 0
    AM_EXPECT_EQ(state.GetEnvironmentFactor(12345), 0.0f);

    // Test with wrapper
    Entity wrapper(&state);
    AM_EXPECT_EQ(wrapper.GetState(), &state);

    constexpr AmEnvironmentID wrapperEnvironment = 2;
    constexpr AmReal32 wrapperFactor = 0.78f;
    wrapper.SetEnvironmentFactor(wrapperEnvironment, wrapperFactor);

    AM_EXPECT_EQ(wrapper.GetEnvironmentFactor(wrapperEnvironment), wrapperFactor);
    AM_EXPECT_EQ(wrapper.GetEnvironmentFactor(wrapperEnvironment), state.GetEnvironmentFactor(wrapperEnvironment));

    // Test that wrapper environments list is updated
    AM_EXPECT_EQ(wrapper.GetEnvironments().size(), 3);
    AM_EXPECT_EQ(wrapper.GetEnvironments().at(environment), factor);
    AM_EXPECT_EQ(wrapper.GetEnvironments().at(wrapperEnvironment), wrapperFactor);

    AM_EXPECT_EQ(wrapper.GetEnvironments().size(), state.GetEnvironments().size());
    AM_EXPECT_EQ(wrapper.GetEnvironments().at(environment), state.GetEnvironmentFactor(environment));
    AM_EXPECT_EQ(wrapper.GetEnvironments().at(wrapperEnvironment), state.GetEnvironmentFactor(wrapperEnvironment));

    // Test unregistered environment returns 0
    AM_EXPECT_EQ(wrapper.GetEnvironmentFactor(12345), 0.0f);
    AM_EXPECT_EQ(wrapper.GetEnvironmentFactor(12345), state.GetEnvironmentFactor(12345));
}
