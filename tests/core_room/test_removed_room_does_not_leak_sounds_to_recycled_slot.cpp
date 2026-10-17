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

#include "EngineTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    // Room states are pooled. Removing a room that a channel is still routed through must detach it right away, or
    // the free list can hand that very state to the next AddRoom() call while it still "owns" a voice that belongs
    // to nobody.
    AM_TEST_CASE(EngineTestCase, core_room, removed_room_does_not_leak_sounds_to_recycled_slot)
    {
    public:
        void Run() override
        {
            Room room = amEngine->AddRoom(500);
            AM_EXPECT(room.Valid());
            room.SetDimensions({ 10.0f, 10.0f, 10.0f });

            // Entity scoped and at the room's location: the engine computes the room of its sounds itself since
            // this fixture does not track rooms.
            Entity entity = amEngine->AddEntity(600);
            AM_EXPECT(entity.Valid());
            entity.SetLocation({ 0.0f, 0.0f, 0.0f });

            Channel channel = amEngine->Play(amEngine->GetCollectionHandle("grass_footsteps"), entity);
            amEngine->WaitUntilFrames(2);
            AM_EXPECT(channel.Valid());
            AM_EXPECT(channel.GetRoom().Valid());
            AM_EXPECT_EQ(channel.GetRoom().GetId(), room.GetId());

            // Paused: keeps the voice (and its attachment to the room) alive for the rest of the test.
            channel.Pause();

            RoomInternalState* state = room.GetState();
            AM_EXPECT_EQ(state->GetPlayingSoundList().size(), static_cast<AmSize>(1));

            amEngine->RemoveRoom(500);

            // The just-removed state must not still list the channel: nothing stops the free list from handing
            // this very slot to the next AddRoom() call.
            AM_EXPECT_EQ(state->GetPlayingSoundList().size(), static_cast<AmSize>(0));

            channel.Stop(0.0);
            amEngine->RemoveEntity(600);
        }
    };

    AM_REGISTER_TEST(core_room, removed_room_does_not_leak_sounds_to_recycled_slot);
} // namespace SparkyStudios::Audio::Amplitude::Tests
