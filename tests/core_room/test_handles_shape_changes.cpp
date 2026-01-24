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
    AM_TEST_CASE(SimpleTestCase, core_room, handles_shape_changes)
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

            const BoxShape shape(500, 250, 250);
            state.SetShape(shape);

            AM_EXPECT_EQ(state.GetShape(), shape);
            AM_EXPECT_EQ(wrapper.GetShape(), shape);

            wrapper.SetShape(shape);

            AM_EXPECT_EQ(state.GetShape(), shape);
            AM_EXPECT_EQ(wrapper.GetShape(), shape);

            AM_EXPECT_EQ(state.GetDimensions(), (AmVector3{ 1000, 500, 500 }));
            AM_EXPECT_EQ(wrapper.GetDimensions(), (AmVector3{ 1000, 500, 500 }));

            AM_EXPECT_EQ(state.GetVolume(), 1000 * 500 * 500);
            AM_EXPECT_EQ(wrapper.GetVolume(), 1000 * 500 * 500);

            AM_EXPECT_EQ(state.GetSurfaceArea(eRoomWall_Back), 1000 * 500);
            AM_EXPECT_EQ(state.GetSurfaceArea(eRoomWall_Front), state.GetSurfaceArea(eRoomWall_Back));
            AM_EXPECT_EQ(state.GetSurfaceArea(eRoomWall_Left), 500 * 500);
            AM_EXPECT_EQ(state.GetSurfaceArea(eRoomWall_Right), state.GetSurfaceArea(eRoomWall_Left));
            AM_EXPECT_EQ(state.GetSurfaceArea(eRoomWall_Top), 1000 * 500);
            AM_EXPECT_EQ(state.GetSurfaceArea(eRoomWall_Bottom), state.GetSurfaceArea(eRoomWall_Top));
            AM_EXPECT_EQ(state.GetSurfaceArea(eRoomWall_Floor), 1000 * 500);
            AM_EXPECT_EQ(state.GetSurfaceArea(eRoomWall_Ceiling), state.GetSurfaceArea(eRoomWall_Floor));

            AM_EXPECT_EQ(wrapper.GetSurfaceArea(eRoomWall_Back), 1000 * 500);
            AM_EXPECT_EQ(wrapper.GetSurfaceArea(eRoomWall_Front), wrapper.GetSurfaceArea(eRoomWall_Back));
            AM_EXPECT_EQ(wrapper.GetSurfaceArea(eRoomWall_Left), 500 * 500);
            AM_EXPECT_EQ(wrapper.GetSurfaceArea(eRoomWall_Right), wrapper.GetSurfaceArea(eRoomWall_Left));
            AM_EXPECT_EQ(wrapper.GetSurfaceArea(eRoomWall_Top), 1000 * 500);
            AM_EXPECT_EQ(wrapper.GetSurfaceArea(eRoomWall_Bottom), wrapper.GetSurfaceArea(eRoomWall_Top));
            AM_EXPECT_EQ(wrapper.GetSurfaceArea(eRoomWall_Floor), 1000 * 500);
            AM_EXPECT_EQ(wrapper.GetSurfaceArea(eRoomWall_Ceiling), wrapper.GetSurfaceArea(eRoomWall_Ceiling));

            AM_EXPECT_EQ(state.GetSurfaceArea((eRoomWall)99), 0.0f);
            AM_EXPECT_EQ(wrapper.GetSurfaceArea((eRoomWall)99), 0.0f);
        }
    };

    AM_REGISTER_TEST(core_room, handles_shape_changes);
} // namespace SparkyStudios::Audio::Amplitude::Tests
