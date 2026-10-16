// Copyright (c) 2026-present Sparky Studios. All rights reserved.
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
#include <Core/RoomInternalState.h>

#include "ComponentTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    // The Engine hands out pooled states: the entity added next must not inherit anything from the one removed.
    AM_TEST_CASE(ComponentTestCase, core_entity, recycled_state_starts_clean)
    {
    public:
        void Run() override
        {
            RoomInternalState roomState;
            roomState.SetId(10);

            fplutil::intrusive_list room_list(&RoomInternalState::node);
            room_list.push_back(roomState);

            EntityInternalState state;
            state.SetId(1);
            state.SetLocation({ 1.0f, 2.0f, 3.0f });
            state.SetObstruction(0.5f);
            state.SetOcclusion(0.25f);
            state.SetRoom(Room(&roomState));
            AM_EXPECT(state.GetRoom().Valid());

            state.Reset();
            state.SetId(2);

            AM_EXPECT_EQ(state.GetId(), 2);
            AM_EXPECT_NOT(state.GetRoom().Valid());
            AM_EXPECT(state.GetLocation() == kVector3Zero);
            AM_EXPECT(state.GetObstruction() == 0.0f);
            AM_EXPECT(state.GetOcclusion() == 0.0f);
        }
    };

    // Rooms are pooled too: once the room an entity was given is removed and its state reused by another room, the
    // entity must not report the new room.
    AM_TEST_CASE(ComponentTestCase, core_entity, room_handle_does_not_follow_a_recycled_room_state)
    {
    public:
        void Run() override
        {
            RoomInternalState roomState;
            roomState.SetId(10);

            fplutil::intrusive_list room_list(&RoomInternalState::node);
            room_list.push_back(roomState);

            EntityInternalState state;
            state.SetId(1);
            state.SetRoom(Room(&roomState));
            AM_EXPECT(state.GetRoom().Valid());
            AM_EXPECT_EQ(state.GetRoom().GetId(), 10);

            roomState.SetId(11);
            AM_EXPECT_NOT(state.GetRoom().Valid());

            // The game gives it the new room: it is reported again.
            state.SetRoom(Room(&roomState));
            AM_EXPECT_EQ(state.GetRoom().GetId(), 11);
        }
    };

    AM_REGISTER_TEST(core_entity, recycled_state_starts_clean);
    AM_REGISTER_TEST(core_entity, room_handle_does_not_follow_a_recycled_room_state);
} // namespace SparkyStudios::Audio::Amplitude::Tests
