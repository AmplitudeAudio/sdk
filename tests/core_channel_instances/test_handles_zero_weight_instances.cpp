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

        // Enable blended mode
        channel.EnableInstancing(eChannelInstanceMode_Blended);

        // Add instances with zero weight
        ChannelInstance instance1 = channel.AddInstance({ 100.0f, 0.0f, 50.0f });
        instance1.SetWeight(0.0f);

        ChannelInstance instance2 = channel.AddInstance({ 120.0f, 0.0f, 80.0f });
        instance2.SetWeight(0.0f);

        // Verify weights are set to zero
        AM_EXPECT(std::abs(instance1.GetWeight() - 0.0f) < kEpsilon);
        AM_EXPECT(std::abs(instance2.GetWeight() - 0.0f) < kEpsilon);

        // The channel should still be valid and playing
        // (though it may be silent due to zero total weight)
        AM_EXPECT(channel.Playing());

        // Add an instance with non-zero weight
        ChannelInstance instance3 = channel.AddInstance({ 140.0f, 0.0f, 110.0f });
        instance3.SetWeight(1.0f);

        AM_EXPECT(std::abs(instance3.GetWeight() - 1.0f) < kEpsilon);

        // Let the sound play
        amEngine->WaitUntilFrames(10);

        // Channel should still be playing
        AM_EXPECT(channel.Playing());
    }
} // namespace SparkyStudios::Audio::Amplitude::Tests
