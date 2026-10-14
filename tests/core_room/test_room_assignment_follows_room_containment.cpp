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

#include "EngineTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(EngineTestCase, core_room, assigns_entity_sounds_to_the_room_containing_them)
    {
    public:
        void Run() override
        {
            AM_EXPECT_NOT(amEngine->IsGameTrackingRooms());

            // The game does not track rooms: the engine finds the room from the entity location.
            Room room = amEngine->AddRoom(100);
            AM_EXPECT(room.Valid());
            room.SetDimensions({ 10.0f, 10.0f, 10.0f });

            Entity entity = amEngine->AddEntity(200);
            AM_EXPECT(entity.Valid());
            entity.SetLocation({ 0.0f, 0.0f, 0.0f });

            Channel channel = amEngine->Play(amEngine->GetCollectionHandle("grass_footsteps"), entity);
            amEngine->WaitUntilFrames(2);
            AM_EXPECT(channel.Valid());

            AM_EXPECT(channel.GetRoom().Valid());
            AM_EXPECT(channel.GetRoom().GetId() == room.GetId());

            // Moving the entity out of the room takes its sounds out of the room as well.
            entity.SetLocation({ 1000.0f, 0.0f, 0.0f });
            amEngine->WaitUntilFrames(2);

            AM_EXPECT(channel.Playing());
            AM_EXPECT_NOT(channel.GetRoom().Valid());

            channel.Stop();
            amEngine->RemoveEntity(200);
            amEngine->RemoveRoom(100);
        }
    };

    AM_TEST_CASE(EngineTestCase, core_room, does_not_assign_the_nearest_room_outside_every_room)
    {
    public:
        void Run() override
        {
            // Two rooms, both far away from the entity: the closest one is still not a room it is in.
            Room room = amEngine->AddRoom(300);
            AM_EXPECT(room.Valid());
            room.SetDimensions({ 10.0f, 10.0f, 10.0f });
            room.SetLocation({ 0.0f, 0.0f, 0.0f });

            Room otherRoom = amEngine->AddRoom(301);
            AM_EXPECT(otherRoom.Valid());
            otherRoom.SetDimensions({ 10.0f, 10.0f, 10.0f });
            otherRoom.SetLocation({ 20.0f, 0.0f, 0.0f });

            Entity entity = amEngine->AddEntity(400);
            AM_EXPECT(entity.Valid());

            // Standing in the gap between both rooms, but outside of them: the first room spans [-5, 5],
            // the second one [15, 25].
            entity.SetLocation({ 10.0f, 0.0f, 0.0f });

            Channel channel = amEngine->Play(amEngine->GetCollectionHandle("grass_footsteps"), entity);
            amEngine->WaitUntilFrames(2);
            AM_EXPECT(channel.Valid());

            AM_EXPECT_NOT(channel.GetRoom().Valid());

            // Entering one of the rooms assigns it.
            entity.SetLocation({ 0.0f, 0.0f, 0.0f });
            amEngine->WaitUntilFrames(2);

            AM_EXPECT(channel.GetRoom().Valid());
            AM_EXPECT(channel.GetRoom().GetId() == room.GetId());

            channel.Stop();
            amEngine->RemoveEntity(400);
            amEngine->RemoveRoom(300);
            amEngine->RemoveRoom(301);
        }
    };

    AM_REGISTER_TEST(core_room, assigns_entity_sounds_to_the_room_containing_them);
    AM_REGISTER_TEST(core_room, does_not_assign_the_nearest_room_outside_every_room);
} // namespace SparkyStudios::Audio::Amplitude::Tests