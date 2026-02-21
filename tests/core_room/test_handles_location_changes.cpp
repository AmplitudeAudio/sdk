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

#include "ComponentTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(ComponentTestCase, core_room, handles_location_changes)
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

            {
                constexpr AmVector3 location = { 10, 20, 30 };
                state.SetLocation(location);

                AM_EXPECT_EQ(state.GetLocation(), location);
                AM_EXPECT_EQ(wrapper.GetLocation(), location);

                state.Update();

                AM_EXPECT_EQ(state.GetLocation(), location);
                AM_EXPECT_EQ(wrapper.GetLocation(), location);
            }

            {
                constexpr AmVector3 location = { 30, 20, 10 };
                wrapper.SetLocation(location);

                AM_EXPECT_EQ(state.GetLocation(), location);
                AM_EXPECT_EQ(wrapper.GetLocation(), location);

                wrapper.Update();

                AM_EXPECT_EQ(state.GetLocation(), location);
                AM_EXPECT_EQ(wrapper.GetLocation(), location);
            }
        }
    };

    AM_REGISTER_TEST(core_room, handles_location_changes);
} // namespace SparkyStudios::Audio::Amplitude::Tests
