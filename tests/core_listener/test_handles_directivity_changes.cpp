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

#include <Core/ListenerInternalState.h>

#include "SimpleTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(SimpleTestCase, core_listener, handles_directivity_changes)
    {
    public:
        void Run() override
        {
            ListenerInternalState state;
            state.SetId(1);

            fplutil::intrusive_list listener_list(&ListenerInternalState::node);
            listener_list.push_back(state);

            Listener wrapper(&state);
            AM_EXPECT_EQ(wrapper.GetState(), &state);

            constexpr auto directivity = 0.5f;
            constexpr auto sharpness = 1.5f;
            state.SetDirectivity(directivity, sharpness);

            AM_EXPECT_EQ(state.GetDirectivity(), directivity);
            AM_EXPECT_EQ(state.GetDirectivitySharpness(), sharpness);

            AM_EXPECT_EQ(wrapper.GetDirectivity(), directivity);
            AM_EXPECT_EQ(wrapper.GetDirectivitySharpness(), sharpness);
        }
    };

    AM_REGISTER_TEST(core_listener, handles_directivity_changes);
} // namespace SparkyStudios::Audio::Amplitude::Tests
