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

        // Add some instances
        ChannelInstance instance1 = channel.AddInstance({ 100.0f, 0.0f, 50.0f });
        ChannelInstance instance2 = channel.AddInstance({ 120.0f, 0.0f, 80.0f });

        AM_EXPECT(channel.GetInstanceCount() == 2);

        // Try to remove a non-existent instance - should not crash
        channel.RemoveInstance(999999);

        // Instance count should remain unchanged
        AM_EXPECT(channel.GetInstanceCount() == 2);

        // Try to remove the same instance twice
        channel.RemoveInstance(instance1.GetId());
        AM_EXPECT(channel.GetInstanceCount() == 1);

        // Removing again should not crash or change count
        channel.RemoveInstance(instance1.GetId());
        AM_EXPECT(channel.GetInstanceCount() == 1);
    }
} // namespace SparkyStudios::Audio::Amplitude::Tests
