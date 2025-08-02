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
    const auto lastLocation = state.GetLocation();
    const AmVector3 location = { 10, 20, 30 };
    state.SetLocation(location);

    AM_EXPECT_EQ(state.GetLocation(), location);

    state.Update();

    const auto& velocity = Sub(location, lastLocation);
    AM_EXPECT_EQ(state.GetVelocity(), velocity);

    // Test with wrapper
    Entity wrapper(&state);
    AM_EXPECT_EQ(wrapper.GetState(), &state);

    const auto lastWrapperLocation = state.GetLocation();
    const auto wrapperLocation = AmVector3{ 100, 200, 300 };
    wrapper.SetLocation(wrapperLocation);

    AM_EXPECT_EQ(wrapper.GetLocation(), wrapperLocation);
    AM_EXPECT_EQ(wrapper.GetLocation(), state.GetLocation());

    wrapper.Update();

    const auto wrapperVelocity = Sub(wrapperLocation, lastWrapperLocation);
    AM_EXPECT_EQ(wrapper.GetVelocity(), wrapperVelocity);
    AM_EXPECT_EQ(wrapper.GetVelocity(), state.GetVelocity());
}
