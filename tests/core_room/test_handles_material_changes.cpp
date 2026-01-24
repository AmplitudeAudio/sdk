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
    AM_TEST_CASE(SimpleTestCase, core_room, handles_material_changes)
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
                state.SetWallMaterial(eRoomWall_Front, RoomWallMaterial(eRoomWallMaterialType_Metal));
                state.SetWallMaterial(eRoomWall_Back, RoomWallMaterial(eRoomWallMaterialType_Marble));

                AM_EXPECT_EQ(state.GetWallMaterial(eRoomWall_Front), RoomWallMaterial(eRoomWallMaterialType_Metal));
                AM_EXPECT_EQ(wrapper.GetWallMaterial(eRoomWall_Front), RoomWallMaterial(eRoomWallMaterialType_Metal));

                AM_EXPECT_EQ(state.GetWallMaterial(eRoomWall_Back), RoomWallMaterial(eRoomWallMaterialType_Marble));
                AM_EXPECT_EQ(wrapper.GetWallMaterial(eRoomWall_Back), RoomWallMaterial(eRoomWallMaterialType_Marble));

                AM_EXPECT_EQ(state.GetWallMaterial(eRoomWall_Bottom), RoomWallMaterial(eRoomWallMaterialType_Transparent));
                AM_EXPECT_EQ(wrapper.GetWallMaterial(eRoomWall_Bottom), RoomWallMaterial(eRoomWallMaterialType_Transparent));

                AM_EXPECT_EQ(state.GetWallMaterial(eRoomWall_Top), RoomWallMaterial(eRoomWallMaterialType_Transparent));
                AM_EXPECT_EQ(wrapper.GetWallMaterial(eRoomWall_Top), RoomWallMaterial(eRoomWallMaterialType_Transparent));

                AM_EXPECT_EQ(state.GetWallMaterial(eRoomWall_Left), RoomWallMaterial(eRoomWallMaterialType_Transparent));
                AM_EXPECT_EQ(wrapper.GetWallMaterial(eRoomWall_Left), RoomWallMaterial(eRoomWallMaterialType_Transparent));

                AM_EXPECT_EQ(state.GetWallMaterial(eRoomWall_Right), RoomWallMaterial(eRoomWallMaterialType_Transparent));
                AM_EXPECT_EQ(wrapper.GetWallMaterial(eRoomWall_Right), RoomWallMaterial(eRoomWallMaterialType_Transparent));
            }

            {
                wrapper.SetWallMaterial(eRoomWall_Left, RoomWallMaterial(eRoomWallMaterialType_Wood));
                wrapper.SetWallMaterial(eRoomWall_Right, RoomWallMaterial(eRoomWallMaterialType_Glass));

                AM_EXPECT_EQ(state.GetWallMaterial(eRoomWall_Left), RoomWallMaterial(eRoomWallMaterialType_Wood));
                AM_EXPECT_EQ(wrapper.GetWallMaterial(eRoomWall_Left), RoomWallMaterial(eRoomWallMaterialType_Wood));

                AM_EXPECT_EQ(state.GetWallMaterial(eRoomWall_Right), RoomWallMaterial(eRoomWallMaterialType_Glass));
                AM_EXPECT_EQ(wrapper.GetWallMaterial(eRoomWall_Right), RoomWallMaterial(eRoomWallMaterialType_Glass));

                AM_EXPECT_EQ(state.GetWallMaterial(eRoomWall_Front), RoomWallMaterial(eRoomWallMaterialType_Metal));
                AM_EXPECT_EQ(wrapper.GetWallMaterial(eRoomWall_Front), RoomWallMaterial(eRoomWallMaterialType_Metal));

                AM_EXPECT_EQ(state.GetWallMaterial(eRoomWall_Back), RoomWallMaterial(eRoomWallMaterialType_Marble));
                AM_EXPECT_EQ(wrapper.GetWallMaterial(eRoomWall_Back), RoomWallMaterial(eRoomWallMaterialType_Marble));

                AM_EXPECT_EQ(state.GetWallMaterial(eRoomWall_Bottom), RoomWallMaterial(eRoomWallMaterialType_Transparent));
                AM_EXPECT_EQ(wrapper.GetWallMaterial(eRoomWall_Bottom), RoomWallMaterial(eRoomWallMaterialType_Transparent));

                AM_EXPECT_EQ(state.GetWallMaterial(eRoomWall_Top), RoomWallMaterial(eRoomWallMaterialType_Transparent));
                AM_EXPECT_EQ(wrapper.GetWallMaterial(eRoomWall_Top), RoomWallMaterial(eRoomWallMaterialType_Transparent));
            }
        }
    };

    AM_REGISTER_TEST(core_room, handles_material_changes);
} // namespace SparkyStudios::Audio::Amplitude::Tests
