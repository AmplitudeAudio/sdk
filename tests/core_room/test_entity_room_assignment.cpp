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
    class TrackRoomsEngineTestCase : public EngineTestCase
    {
    public:
        [[nodiscard]] const AmOsChar* GetConfigFile() const override
        {
            return AM_OS_STRING("tests.track_rooms.config.amconfig");
        }
    };

    AM_TEST_CASE(TrackRoomsEngineTestCase, core_room, entity_room_drives_the_room_of_its_sounds)
    {
    public:
        void Run() override
        {
            AM_EXPECT(amEngine->IsGameTrackingRooms());

            Room room = amEngine->AddRoom(100);
            AM_EXPECT(room.Valid());
            room.SetDimensions({ 10.0f, 10.0f, 10.0f });

            Room otherRoom = amEngine->AddRoom(101);
            AM_EXPECT(otherRoom.Valid());
            otherRoom.SetDimensions({ 10.0f, 10.0f, 10.0f });
            otherRoom.SetLocation({ 1000.0f, 0.0f, 0.0f });

            Entity entity = amEngine->AddEntity(200);
            AM_EXPECT(entity.Valid());
            entity.SetLocation({ 0.0f, 0.0f, 0.0f });

            // The entity is inside a room, but the game is in charge: no room is assigned on its own.
            AM_EXPECT_NOT(entity.GetRoom().Valid());

            Channel channel = amEngine->Play(amEngine->GetCollectionHandle("grass_footsteps"), entity);
            amEngine->WaitUntilFrames(2);
            AM_EXPECT(channel.Valid());
            AM_EXPECT_NOT(channel.GetRoom().Valid());

            // The room of the entity is the one its sounds are routed through, wherever the entity is.
            entity.SetRoom(otherRoom);

            Channel otherChannel = amEngine->Play(amEngine->GetCollectionHandle("grass_footsteps"), entity);
            amEngine->WaitUntilFrames(2);
            AM_EXPECT(otherChannel.Valid());
            AM_EXPECT(otherChannel.GetRoom().Valid());
            AM_EXPECT(otherChannel.GetRoom().GetId() == otherRoom.GetId());

            // The already playing channel follows the entity room as well.
            amEngine->WaitUntilFrames(2);
            AM_EXPECT(channel.GetRoom().Valid());
            AM_EXPECT(channel.GetRoom().GetId() == otherRoom.GetId());

            // Moving the entity inside another room does not change anything: the game is in charge.
            entity.SetLocation({ 0.0f, 0.0f, 0.0f });
            amEngine->WaitUntilFrames(2);
            AM_EXPECT(channel.GetRoom().GetId() == otherRoom.GetId());

            // Clearing the room of the entity takes its sounds out of any room.
            entity.SetRoom(Room(nullptr));
            AM_EXPECT_NOT(entity.GetRoom().Valid());

            amEngine->WaitUntilFrames(2);
            AM_EXPECT_NOT(channel.GetRoom().Valid());
            AM_EXPECT_NOT(otherChannel.GetRoom().Valid());

            channel.Stop();
            otherChannel.Stop();
            amEngine->RemoveEntity(200);
            amEngine->RemoveRoom(100);
            amEngine->RemoveRoom(101);
        }
    };

    AM_TEST_CASE(EngineTestCase, core_room, engine_overrides_the_entity_room_when_the_game_does_not_track_rooms)
    {
    public:
        void Run() override
        {
            AM_EXPECT_NOT(amEngine->IsGameTrackingRooms());

            Room room = amEngine->AddRoom(300);
            AM_EXPECT(room.Valid());
            room.SetDimensions({ 10.0f, 10.0f, 10.0f });

            Entity entity = amEngine->AddEntity(400);
            AM_EXPECT(entity.Valid());
            entity.SetLocation({ 0.0f, 0.0f, 0.0f });

            // The game may store a room on the entity, but the engine computes the room of its sounds itself.
            Room otherRoom = amEngine->AddRoom(301);
            AM_EXPECT(otherRoom.Valid());
            otherRoom.SetDimensions({ 10.0f, 10.0f, 10.0f });
            otherRoom.SetLocation({ 1000.0f, 0.0f, 0.0f });
            entity.SetRoom(otherRoom);

            Channel channel = amEngine->Play(amEngine->GetCollectionHandle("grass_footsteps"), entity);
            amEngine->WaitUntilFrames(2);
            AM_EXPECT(channel.Valid());

            AM_EXPECT(entity.GetRoom().GetId() == otherRoom.GetId());
            AM_EXPECT(channel.GetRoom().Valid());
            AM_EXPECT(channel.GetRoom().GetId() == room.GetId());

            channel.Stop();
            amEngine->RemoveEntity(400);
            amEngine->RemoveRoom(300);
            amEngine->RemoveRoom(301);
        }
    };

    AM_REGISTER_TEST(core_room, entity_room_drives_the_room_of_its_sounds);
    AM_REGISTER_TEST(core_room, engine_overrides_the_entity_room_when_the_game_does_not_track_rooms);
} // namespace SparkyStudios::Audio::Amplitude::Tests