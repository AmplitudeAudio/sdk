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

#include <Core/RoomInternalState.h>

#include "SimpleTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(SimpleTestCase, core_room, handles_cutoff_frequency_changes)
    {
    public:
        void Run() override
        {
            RoomInternalState state;
            state.SetId(1);

            fplutil::intrusive_list room_list(&RoomInternalState::node);
            room_list.push_back(state);

            Room wrapper(&state);
            AM_EXPECT_EQ(wrapper.GetState(), &state);

            constexpr auto kCutoffFrequency = 1000.0f;
            state.SetCutOffFrequency(kCutoffFrequency);

            AM_EXPECT_EQ(state.GetCutOffFrequency(), kCutoffFrequency);
        }
    };

    AM_REGISTER_TEST(core_room, handles_cutoff_frequency_changes);
} // namespace SparkyStudios::Audio::Amplitude::Tests
