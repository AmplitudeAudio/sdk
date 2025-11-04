// Copyright (c) 2025-present Sparky Studios. All rights reserved.
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

using namespace SparkyStudios::Audio::Amplitude;

void SimpleTestCase::Run()
{
    RoomInternalState state;
    state.SetId(1);

    fplutil::intrusive_list room_list(&RoomInternalState::node);
    room_list.push_back(state);

    Room wrapper(&state);
    AM_EXPECT_EQ(wrapper.GetState(), &state);

    // Test SetAllWallMaterials sets all walls to the same material
    {
        RoomWallMaterial metalMaterial(eRoomWallMaterialType_Metal);
        wrapper.SetAllWallMaterials(metalMaterial);

        // Verify all walls have the metal material
        AM_EXPECT_EQ(wrapper.GetWallMaterial(eRoomWall_Left), metalMaterial);
        AM_EXPECT_EQ(wrapper.GetWallMaterial(eRoomWall_Right), metalMaterial);
        AM_EXPECT_EQ(wrapper.GetWallMaterial(eRoomWall_Floor), metalMaterial);
        AM_EXPECT_EQ(wrapper.GetWallMaterial(eRoomWall_Ceiling), metalMaterial);
        AM_EXPECT_EQ(wrapper.GetWallMaterial(eRoomWall_Front), metalMaterial);
        AM_EXPECT_EQ(wrapper.GetWallMaterial(eRoomWall_Back), metalMaterial);

        // Also verify through state
        AM_EXPECT_EQ(state.GetWallMaterial(eRoomWall_Left), metalMaterial);
        AM_EXPECT_EQ(state.GetWallMaterial(eRoomWall_Right), metalMaterial);
        AM_EXPECT_EQ(state.GetWallMaterial(eRoomWall_Floor), metalMaterial);
        AM_EXPECT_EQ(state.GetWallMaterial(eRoomWall_Ceiling), metalMaterial);
        AM_EXPECT_EQ(state.GetWallMaterial(eRoomWall_Front), metalMaterial);
        AM_EXPECT_EQ(state.GetWallMaterial(eRoomWall_Back), metalMaterial);
    }

    // Test changing to a different material for all walls
    {
        RoomWallMaterial glassMaterial(eRoomWallMaterialType_Glass);
        wrapper.SetAllWallMaterials(glassMaterial);

        // Verify all walls now have the glass material
        AM_EXPECT_EQ(wrapper.GetWallMaterial(eRoomWall_Left), glassMaterial);
        AM_EXPECT_EQ(wrapper.GetWallMaterial(eRoomWall_Right), glassMaterial);
        AM_EXPECT_EQ(wrapper.GetWallMaterial(eRoomWall_Floor), glassMaterial);
        AM_EXPECT_EQ(wrapper.GetWallMaterial(eRoomWall_Ceiling), glassMaterial);
        AM_EXPECT_EQ(wrapper.GetWallMaterial(eRoomWall_Front), glassMaterial);
        AM_EXPECT_EQ(wrapper.GetWallMaterial(eRoomWall_Back), glassMaterial);

        // Verify the materials are not the previous metal material
        RoomWallMaterial metalMaterial(eRoomWallMaterialType_Metal);
        AM_EXPECT(wrapper.GetWallMaterial(eRoomWall_Left) != metalMaterial);
        AM_EXPECT(wrapper.GetWallMaterial(eRoomWall_Right) != metalMaterial);
        AM_EXPECT(wrapper.GetWallMaterial(eRoomWall_Floor) != metalMaterial);
        AM_EXPECT(wrapper.GetWallMaterial(eRoomWall_Ceiling) != metalMaterial);
        AM_EXPECT(wrapper.GetWallMaterial(eRoomWall_Front) != metalMaterial);
        AM_EXPECT(wrapper.GetWallMaterial(eRoomWall_Back) != metalMaterial);
    }
}
