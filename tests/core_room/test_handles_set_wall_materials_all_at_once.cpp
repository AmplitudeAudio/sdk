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
#include "SparkyStudios/Audio/Amplitude/Core/Room.h"

using namespace SparkyStudios::Audio::Amplitude;

void SimpleTestCase::Run()
{
    RoomInternalState state;
    state.SetId(1);

    fplutil::intrusive_list room_list(&RoomInternalState::node);
    room_list.push_back(state);

    Room wrapper(&state);
    AM_EXPECT_EQ(wrapper.GetState(), &state);

    // Test SetWallMaterials sets all six walls to different materials in one call
    {
        RoomWallMaterial leftMaterial(eRoomWallMaterialType_Wood);
        RoomWallMaterial rightMaterial(eRoomWallMaterialType_Metal);
        RoomWallMaterial floorMaterial(eRoomWallMaterialType_Marble);
        RoomWallMaterial ceilingMaterial(eRoomWallMaterialType_Glass);
        RoomWallMaterial frontMaterial(eRoomWallMaterialType_Transparent);
        RoomWallMaterial backMaterial(eRoomWallMaterialType_ConcreteUnpainted);

        wrapper.SetWallMaterials(leftMaterial, rightMaterial, floorMaterial, ceilingMaterial, frontMaterial, backMaterial);

        // Verify each wall has the correct material through wrapper
        AM_EXPECT_EQ(wrapper.GetWallMaterial(eRoomWall_Left), leftMaterial);
        AM_EXPECT_EQ(wrapper.GetWallMaterial(eRoomWall_Right), rightMaterial);
        AM_EXPECT_EQ(wrapper.GetWallMaterial(eRoomWall_Floor), floorMaterial);
        AM_EXPECT_EQ(wrapper.GetWallMaterial(eRoomWall_Ceiling), ceilingMaterial);
        AM_EXPECT_EQ(wrapper.GetWallMaterial(eRoomWall_Front), frontMaterial);
        AM_EXPECT_EQ(wrapper.GetWallMaterial(eRoomWall_Back), backMaterial);

        // Also verify through state
        AM_EXPECT_EQ(state.GetWallMaterial(eRoomWall_Left), leftMaterial);
        AM_EXPECT_EQ(state.GetWallMaterial(eRoomWall_Right), rightMaterial);
        AM_EXPECT_EQ(state.GetWallMaterial(eRoomWall_Floor), floorMaterial);
        AM_EXPECT_EQ(state.GetWallMaterial(eRoomWall_Ceiling), ceilingMaterial);
        AM_EXPECT_EQ(state.GetWallMaterial(eRoomWall_Front), frontMaterial);
        AM_EXPECT_EQ(state.GetWallMaterial(eRoomWall_Back), backMaterial);
    }

    // Test SetWallMaterials with same materials for some walls
    {
        RoomWallMaterial commonMaterial(eRoomWallMaterialType_Wood);
        RoomWallMaterial specialMaterial(eRoomWallMaterialType_Glass);

        wrapper.SetWallMaterials(
            commonMaterial, // left
            commonMaterial, // right
            specialMaterial, // floor
            specialMaterial, // ceiling
            commonMaterial, // front
            commonMaterial // back
        );

        // Verify materials are set correctly
        AM_EXPECT_EQ(wrapper.GetWallMaterial(eRoomWall_Left), commonMaterial);
        AM_EXPECT_EQ(wrapper.GetWallMaterial(eRoomWall_Right), commonMaterial);
        AM_EXPECT_EQ(wrapper.GetWallMaterial(eRoomWall_Floor), specialMaterial);
        AM_EXPECT_EQ(wrapper.GetWallMaterial(eRoomWall_Ceiling), specialMaterial);
        AM_EXPECT_EQ(wrapper.GetWallMaterial(eRoomWall_Front), commonMaterial);
        AM_EXPECT_EQ(wrapper.GetWallMaterial(eRoomWall_Back), commonMaterial);

        // Verify common materials are equal to each other
        AM_EXPECT_EQ(wrapper.GetWallMaterial(eRoomWall_Left), wrapper.GetWallMaterial(eRoomWall_Right));
        AM_EXPECT_EQ(wrapper.GetWallMaterial(eRoomWall_Left), wrapper.GetWallMaterial(eRoomWall_Front));
        AM_EXPECT_EQ(wrapper.GetWallMaterial(eRoomWall_Left), wrapper.GetWallMaterial(eRoomWall_Back));

        // Verify special materials are equal to each other
        AM_EXPECT_EQ(wrapper.GetWallMaterial(eRoomWall_Floor), wrapper.GetWallMaterial(eRoomWall_Ceiling));

        // Verify common and special materials are different
        AM_EXPECT(wrapper.GetWallMaterial(eRoomWall_Left) != wrapper.GetWallMaterial(eRoomWall_Floor));
    }
}
