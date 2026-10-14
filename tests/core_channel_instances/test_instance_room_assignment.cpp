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
    class TrackRoomsInstanceTestCase : public EngineTestCase
    {
    public:
        [[nodiscard]] const AmOsChar* GetConfigFile() const override
        {
            return AM_OS_STRING("tests.track_rooms.config.amconfig");
        }
    };

    AM_TEST_CASE(EngineTestCase, core_channel_instances, instance_gets_the_room_it_is_in)
    {
    public:
        void Run() override
        {
            AM_EXPECT_NOT(amEngine->IsGameTrackingRooms());

            // Two rooms, far from each other.
            Room room = amEngine->AddRoom(100);
            AM_EXPECT(room.Valid());
            room.SetDimensions({ 10.0f, 10.0f, 10.0f });

            Room otherRoom = amEngine->AddRoom(101);
            AM_EXPECT(otherRoom.Valid());
            otherRoom.SetDimensions({ 10.0f, 10.0f, 10.0f });
            otherRoom.SetLocation({ 1000.0f, 0.0f, 0.0f });

            // The entity is far from both rooms: the channel itself is in no room.
            Entity entity = amEngine->AddEntity(200);
            AM_EXPECT(entity.Valid());
            entity.SetLocation({ 500.0f, 0.0f, 0.0f });

            Channel channel = amEngine->Play(amEngine->GetCollectionHandle("grass_footsteps"), entity);
            amEngine->WaitUntilFrames(2);
            AM_EXPECT(channel.Valid());
            AM_EXPECT_NOT(channel.GetRoom().Valid());

            channel.EnableInstancing(eChannelInstanceMode_Separate);

            // Instances are rendered one by one, each with its own reverb: each one is assigned the room
            // containing its own location, which is not the room of the channel.
            ChannelInstance insideRoom = channel.AddInstance({ 0.0f, 0.0f, 0.0f });
            AM_EXPECT(insideRoom.Valid());

            ChannelInstance insideOtherRoom = channel.AddInstance({ 1000.0f, 0.0f, 0.0f });
            AM_EXPECT(insideOtherRoom.Valid());

            ChannelInstance outsideEveryRoom = channel.AddInstance({ 2000.0f, 0.0f, 0.0f });
            AM_EXPECT(outsideEveryRoom.Valid());

            amEngine->WaitUntilFrames(2);

            AM_EXPECT(insideRoom.GetRoom().Valid());
            AM_EXPECT(insideRoom.GetRoom().GetId() == room.GetId());

            AM_EXPECT(insideOtherRoom.GetRoom().Valid());
            AM_EXPECT(insideOtherRoom.GetRoom().GetId() == otherRoom.GetId());

            AM_EXPECT_NOT(outsideEveryRoom.GetRoom().Valid());

            // Moving an instance moves it to the room it enters.
            outsideEveryRoom.SetLocation({ 0.0f, 0.0f, 0.0f });
            amEngine->WaitUntilFrames(2);

            AM_EXPECT(outsideEveryRoom.GetRoom().Valid());
            AM_EXPECT(outsideEveryRoom.GetRoom().GetId() == room.GetId());

            // Leaving it again takes it out of any room.
            outsideEveryRoom.SetLocation({ 2000.0f, 0.0f, 0.0f });
            amEngine->WaitUntilFrames(2);

            AM_EXPECT_NOT(outsideEveryRoom.GetRoom().Valid());

            channel.Stop();
            amEngine->RemoveEntity(200);
            amEngine->RemoveRoom(100);
            amEngine->RemoveRoom(101);
        }
    };

    AM_TEST_CASE(EngineTestCase, core_channel_instances, instance_room_set_by_the_game_is_replaced_when_game_does_not_track_rooms)
    {
    public:
        void Run() override
        {
            Room room = amEngine->AddRoom(300);
            AM_EXPECT(room.Valid());
            room.SetDimensions({ 10.0f, 10.0f, 10.0f });

            Entity entity = amEngine->AddEntity(400);
            AM_EXPECT(entity.Valid());
            entity.SetLocation({ 0.0f, 0.0f, 0.0f });

            Channel channel = amEngine->Play(amEngine->GetCollectionHandle("grass_footsteps"), entity);
            amEngine->WaitUntilFrames(2);
            AM_EXPECT(channel.Valid());

            channel.EnableInstancing(eChannelInstanceMode_Separate);

            ChannelInstance instance = channel.AddInstance({ 500.0f, 0.0f, 0.0f });
            AM_EXPECT(instance.Valid());

            // The game is not tracking rooms: a room set on the instance is replaced by the one the instance
            // is in, which is none here.
            instance.SetRoom(room);
            AM_EXPECT(instance.GetRoom().Valid());

            amEngine->WaitUntilFrames(2);
            AM_EXPECT_NOT(instance.GetRoom().Valid());

            instance.SetLocation({ 0.0f, 0.0f, 0.0f });
            amEngine->WaitUntilFrames(2);

            AM_EXPECT(instance.GetRoom().Valid());
            AM_EXPECT(instance.GetRoom().GetId() == room.GetId());

            channel.Stop();
            amEngine->RemoveEntity(400);
            amEngine->RemoveRoom(300);
        }
    };

    AM_TEST_CASE(TrackRoomsInstanceTestCase, core_channel_instances, instance_follows_the_entity_room_when_game_tracks_rooms)
    {
    public:
        void Run() override
        {
            AM_EXPECT(amEngine->IsGameTrackingRooms());

            // A room to follow, and one the instances must not pick up from their own location.
            Room entityRoom = amEngine->AddRoom(500);
            AM_EXPECT(entityRoom.Valid());
            entityRoom.SetDimensions({ 10.0f, 10.0f, 10.0f });

            Room locatedRoom = amEngine->AddRoom(501);
            AM_EXPECT(locatedRoom.Valid());
            locatedRoom.SetDimensions({ 10.0f, 10.0f, 10.0f });
            locatedRoom.SetLocation({ 0.0f, 0.0f, 0.0f });

            Entity entity = amEngine->AddEntity(600);
            AM_EXPECT(entity.Valid());
            entity.SetLocation({ 1000.0f, 0.0f, 0.0f });

            Channel channel = amEngine->Play(amEngine->GetCollectionHandle("grass_footsteps"), entity);
            amEngine->WaitUntilFrames(2);
            AM_EXPECT(channel.Valid());

            channel.EnableInstancing(eChannelInstanceMode_Separate);

            ChannelInstance instance = channel.AddInstance({ 0.0f, 0.0f, 0.0f });
            AM_EXPECT(instance.Valid());

            // The instance sits inside a room, but the game is in charge: nothing is assigned on its own.
            amEngine->WaitUntilFrames(2);
            AM_EXPECT_NOT(instance.GetRoom().Valid());

            entity.SetRoom(entityRoom);

            amEngine->WaitUntilFrames(2);
            AM_EXPECT(instance.GetRoom().Valid());
            AM_EXPECT(instance.GetRoom().GetId() == entityRoom.GetId());

            // A room set on the instance itself is the game speaking for that instance: it wins.
            instance.SetRoom(locatedRoom);
            amEngine->WaitUntilFrames(2);
            AM_EXPECT(instance.GetRoom().GetId() == locatedRoom.GetId());

            // Clearing the room of the entity takes the instances without a room of their own out of any room.
            entity.SetRoom(Room(nullptr));
            amEngine->WaitUntilFrames(2);

            AM_EXPECT_NOT(channel.GetRoom().Valid());
            AM_EXPECT(instance.GetRoom().Valid());
            AM_EXPECT(instance.GetRoom().GetId() == locatedRoom.GetId());

            channel.Stop();
            amEngine->RemoveEntity(600);
            amEngine->RemoveRoom(500);
            amEngine->RemoveRoom(501);
        }
    };

    AM_REGISTER_TEST(core_channel_instances, instance_gets_the_room_it_is_in);
    AM_REGISTER_TEST(core_channel_instances, instance_room_set_by_the_game_is_replaced_when_game_does_not_track_rooms);
    AM_REGISTER_TEST(core_channel_instances, instance_follows_the_entity_room_when_game_tracks_rooms);
} // namespace SparkyStudios::Audio::Amplitude::Tests