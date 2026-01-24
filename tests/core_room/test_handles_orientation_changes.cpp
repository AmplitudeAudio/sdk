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
    AM_TEST_CASE(SimpleTestCase, core_room, handles_orientation_changes)
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
                const auto direction = kVector3UnitX;
                const auto up = kVector3UnitZ;
                const auto orientation = Orientation(direction, up);
                state.SetOrientation(orientation);

                AM_EXPECT_EQ(state.GetDirection(), direction);
                AM_EXPECT_EQ(wrapper.GetDirection(), direction);

                AM_EXPECT_EQ(state.GetUp(), up);
                AM_EXPECT_EQ(wrapper.GetUp(), up);

                AM_EXPECT(std::memcmp(&state.GetOrientation(), &orientation, sizeof(Orientation)) == 0);
                AM_EXPECT(std::memcmp(&state.GetOrientation(), &orientation, sizeof(Orientation)) == 0);

                AM_EXPECT(std::memcmp(&wrapper.GetOrientation(), &orientation, sizeof(Orientation)) == 0);
                AM_EXPECT(std::memcmp(&wrapper.GetOrientation(), &orientation, sizeof(Orientation)) == 0);

                state.Update();

                AM_EXPECT_EQ(state.GetDirection(), direction);
                AM_EXPECT_EQ(wrapper.GetDirection(), direction);

                AM_EXPECT_EQ(state.GetUp(), up);
                AM_EXPECT_EQ(wrapper.GetUp(), up);

                AM_EXPECT(std::memcmp(&state.GetOrientation(), &orientation, sizeof(Orientation)) == 0);
                AM_EXPECT(std::memcmp(&state.GetOrientation(), &orientation, sizeof(Orientation)) == 0);

                AM_EXPECT(std::memcmp(&wrapper.GetOrientation(), &orientation, sizeof(Orientation)) == 0);
                AM_EXPECT(std::memcmp(&wrapper.GetOrientation(), &orientation, sizeof(Orientation)) == 0);
            }

            {
                const auto direction = kVector3UnitY;
                const auto up = kVector3UnitX;
                const auto orientation = Orientation(direction, up);
                wrapper.SetOrientation(orientation);

                AM_EXPECT_EQ(state.GetDirection(), direction);
                AM_EXPECT_EQ(wrapper.GetDirection(), direction);

                AM_EXPECT_EQ(state.GetUp(), up);
                AM_EXPECT_EQ(wrapper.GetUp(), up);

                AM_EXPECT(std::memcmp(&state.GetOrientation(), &orientation, sizeof(Orientation)) == 0);
                AM_EXPECT(std::memcmp(&state.GetOrientation(), &orientation, sizeof(Orientation)) == 0);

                AM_EXPECT(std::memcmp(&wrapper.GetOrientation(), &orientation, sizeof(Orientation)) == 0);
                AM_EXPECT(std::memcmp(&wrapper.GetOrientation(), &orientation, sizeof(Orientation)) == 0);

                wrapper.Update();

                AM_EXPECT_EQ(state.GetDirection(), direction);
                AM_EXPECT_EQ(wrapper.GetDirection(), direction);

                AM_EXPECT_EQ(state.GetUp(), up);
                AM_EXPECT_EQ(wrapper.GetUp(), up);

                AM_EXPECT(std::memcmp(&state.GetOrientation(), &orientation, sizeof(Orientation)) == 0);
                AM_EXPECT(std::memcmp(&state.GetOrientation(), &orientation, sizeof(Orientation)) == 0);

                AM_EXPECT(std::memcmp(&wrapper.GetOrientation(), &orientation, sizeof(Orientation)) == 0);
                AM_EXPECT(std::memcmp(&wrapper.GetOrientation(), &orientation, sizeof(Orientation)) == 0);
            }
        }
    };

    AM_REGISTER_TEST(core_room, handles_orientation_changes);
} // namespace SparkyStudios::Audio::Amplitude::Tests
