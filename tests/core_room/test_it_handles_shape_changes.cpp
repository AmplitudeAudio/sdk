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
#include "SparkyStudios/Audio/Amplitude/Core/Common/Platforms/UNIX/Types.h"
#include "SparkyStudios/Audio/Amplitude/Core/Room.h"

using namespace SparkyStudios::Audio::Amplitude;

void SimpleTestCase::Run()
{
    RoomInternalState state;
    state.SetId(1);

    fplutil::intrusive_list room_list(&RoomInternalState::node);
    room_list.push_back(state);

    Room wrapper(&state);
    ExpectEqual(wrapper.GetState(), &state);

    BoxShape shape(500, 250, 250);
    state.SetShape(shape);

    ExpectEqual(state.GetShape(), shape, "State should have the correct shape");
    ExpectEqual(wrapper.GetShape(), shape, "Wrapper should have the correct shape");

    ExpectEqual(state.GetDimensions(), AmVector3{ 1000, 500, 500 }, "State should have the correct dimensions");
    ExpectEqual(wrapper.GetDimensions(), AmVector3{ 1000, 500, 500 }, "Wrapper should have the correct dimensions");

    ExpectEqual<AmReal32>(state.GetVolume(), 1000 * 500 * 500, "State should have the correct volume");
    ExpectEqual<AmReal32>(wrapper.GetVolume(), 1000 * 500 * 500, "Wrapper should have the correct volume");

    ExpectEqual<AmReal32>(state.GetSurfaceArea(eRoomWall_Back), 1000 * 500);
    ExpectEqual<AmReal32>(state.GetSurfaceArea(eRoomWall_Front), state.GetSurfaceArea(eRoomWall_Back));
    ExpectEqual<AmReal32>(state.GetSurfaceArea(eRoomWall_Left), 500 * 500);
    ExpectEqual<AmReal32>(state.GetSurfaceArea(eRoomWall_Right), state.GetSurfaceArea(eRoomWall_Left));
    ExpectEqual<AmReal32>(state.GetSurfaceArea(eRoomWall_Top), 1000 * 500);
    ExpectEqual<AmReal32>(state.GetSurfaceArea(eRoomWall_Bottom), state.GetSurfaceArea(eRoomWall_Top));
    ExpectEqual<AmReal32>(state.GetSurfaceArea(eRoomWall_Floor), 1000 * 500);
    ExpectEqual<AmReal32>(state.GetSurfaceArea(eRoomWall_Ceiling), state.GetSurfaceArea(eRoomWall_Ceiling));

    ExpectEqual<AmReal32>(wrapper.GetSurfaceArea(eRoomWall_Back), 1000 * 500);
    ExpectEqual<AmReal32>(wrapper.GetSurfaceArea(eRoomWall_Front), wrapper.GetSurfaceArea(eRoomWall_Back));
    ExpectEqual<AmReal32>(wrapper.GetSurfaceArea(eRoomWall_Left), 500 * 500);
    ExpectEqual<AmReal32>(wrapper.GetSurfaceArea(eRoomWall_Right), wrapper.GetSurfaceArea(eRoomWall_Left));
    ExpectEqual<AmReal32>(wrapper.GetSurfaceArea(eRoomWall_Top), 1000 * 500);
    ExpectEqual<AmReal32>(wrapper.GetSurfaceArea(eRoomWall_Bottom), wrapper.GetSurfaceArea(eRoomWall_Top));
    ExpectEqual<AmReal32>(wrapper.GetSurfaceArea(eRoomWall_Floor), 1000 * 500);
    ExpectEqual<AmReal32>(wrapper.GetSurfaceArea(eRoomWall_Ceiling), wrapper.GetSurfaceArea(eRoomWall_Ceiling));

    constexpr AmVector3 dimensions = { 100, 100, 100 };
    state.SetDimensions(dimensions);

    ExpectEqual(state.GetDimensions(), dimensions);
    ExpectEqual(wrapper.GetDimensions(), dimensions);

    BoxShape boxShape(50, 50, 50);
    ExpectEqual(state.GetShape(), boxShape);
    ExpectEqual(wrapper.GetShape(), boxShape);
}
