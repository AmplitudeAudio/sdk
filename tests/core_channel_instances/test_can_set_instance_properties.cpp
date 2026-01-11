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

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    void EngineTestCase::Run()
    {
        SoundHandle test_sound_01 = amEngine->GetSoundHandle("test_sound_01");

        Channel channel = amEngine->Play(test_sound_01);
        amEngine->WaitUntilFrames(2);

        AM_EXPECT(channel.Valid());

        // Enable instancing
        channel.EnableInstancing(eChannelInstanceMode_Blended);

        // Add an instance
        ChannelInstance instance = channel.AddInstance({ 100.0f, 0.0f, 50.0f });
        AM_EXPECT(instance.Valid());

        // Test location getter
        const AmVector3& location = instance.GetLocation();
        AM_EXPECT(std::abs(location.x - 100.0f) < kEpsilon);
        AM_EXPECT(std::abs(location.y - 0.0f) < kEpsilon);
        AM_EXPECT(std::abs(location.z - 50.0f) < kEpsilon);

        // Test location setter
        instance.SetLocation({ 200.0f, 10.0f, 100.0f });
        const AmVector3& newLocation = instance.GetLocation();
        AM_EXPECT(std::abs(newLocation.x - 200.0f) < kEpsilon);
        AM_EXPECT(std::abs(newLocation.y - 10.0f) < kEpsilon);
        AM_EXPECT(std::abs(newLocation.z - 100.0f) < kEpsilon);

        // Test weight getter (default should be 1.0)
        AmReal32 weight = instance.GetWeight();
        AM_EXPECT(std::abs(weight - 1.0f) < kEpsilon);

        // Test weight setter
        instance.SetWeight(0.75f);
        weight = instance.GetWeight();
        AM_EXPECT(std::abs(weight - 0.75f) < kEpsilon);

        // Test room getter (default should be invalid)
        Room room = instance.GetRoom();
        AM_EXPECT(!room.Valid());

        // Test room setter
        Room testRoom = amEngine->AddRoom(1);
        instance.SetRoom(testRoom);
        room = instance.GetRoom();
        AM_EXPECT(room.Valid());
        AM_EXPECT(room.GetId() == testRoom.GetId());

        // Cleanup
        amEngine->RemoveRoom(1);
    }
} // namespace SparkyStudios::Audio::Amplitude::Tests
