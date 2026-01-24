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
    AM_TEST_CASE(EngineTestCase, core_channel_instances, instances_can_have_different_rooms)
    {
    public:
        void Run() override
        {
            SoundHandle test_sound_01 = amEngine->GetSoundHandle("test_sound_01");

            Channel channel = amEngine->Play(test_sound_01);
            amEngine->WaitUntilFrames(2);

            AM_EXPECT(channel.Valid());

            // Create test rooms
            Room room1 = amEngine->AddRoom(1);
            Room room2 = amEngine->AddRoom(2);
            Room room3 = amEngine->AddRoom(3);

            AM_EXPECT(room1.Valid());
            AM_EXPECT(room2.Valid());
            AM_EXPECT(room3.Valid());

            // Enable instancing
            channel.EnableInstancing(eChannelInstanceMode_Blended);

            // Add instances with different rooms
            ChannelInstance instance1 = channel.AddInstance({ 100.0f, 0.0f, 50.0f });
            instance1.SetRoom(room1);

            ChannelInstance instance2 = channel.AddInstance({ 120.0f, 0.0f, 80.0f });
            instance2.SetRoom(room2);

            ChannelInstance instance3 = channel.AddInstance({ 140.0f, 0.0f, 110.0f });
            instance3.SetRoom(room3);

            // Verify each instance has the correct room
            AM_EXPECT(instance1.GetRoom().Valid());
            AM_EXPECT(instance1.GetRoom().GetId() == room1.GetId());

            AM_EXPECT(instance2.GetRoom().Valid());
            AM_EXPECT(instance2.GetRoom().GetId() == room2.GetId());

            AM_EXPECT(instance3.GetRoom().Valid());
            AM_EXPECT(instance3.GetRoom().GetId() == room3.GetId());

            // Cleanup
            amEngine->RemoveRoom(1);
            amEngine->RemoveRoom(2);
            amEngine->RemoveRoom(3);
        }
    };

    AM_REGISTER_TEST(core_channel_instances, instances_can_have_different_rooms);
} // namespace SparkyStudios::Audio::Amplitude::Tests
