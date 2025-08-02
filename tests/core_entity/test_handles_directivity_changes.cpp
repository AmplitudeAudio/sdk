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
    constexpr auto directivity = 0.5f;
    constexpr auto sharpness = 1.5f;
    state.SetDirectivity(directivity, sharpness);

    AM_EXPECT_EQ(state.GetDirectivity(), directivity);
    AM_EXPECT_EQ(state.GetDirectivitySharpness(), sharpness);

    // Test with wrapper
    Entity wrapper(&state);
    AM_EXPECT_EQ(wrapper.GetState(), &state);

    constexpr auto wrapperDirectivity = 0.8f;
    constexpr auto wrapperSharpness = 2.0f;
    wrapper.SetDirectivity(wrapperDirectivity, wrapperSharpness);

    AM_EXPECT_EQ(wrapper.GetDirectivity(), wrapperDirectivity);
    AM_EXPECT_EQ(wrapper.GetDirectivitySharpness(), wrapperSharpness);

    AM_EXPECT_EQ(wrapper.GetDirectivity(), state.GetDirectivity());
    AM_EXPECT_EQ(wrapper.GetDirectivitySharpness(), state.GetDirectivitySharpness());
}
