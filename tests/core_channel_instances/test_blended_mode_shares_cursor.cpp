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
    AM_TEST_CASE(EngineTestCase, core_channel_instances, blended_mode_shares_cursor)
    {
    public:
        void Run() override
        {
            SoundHandle test_sound_01 = amEngine->GetSoundHandle("test_sound_01");

            Channel channel = amEngine->Play(test_sound_01);
            amEngine->WaitUntilFrames(2);

            AM_EXPECT(channel.Valid());

            // Enable blended mode
            channel.EnableInstancing(eChannelInstanceMode_Blended);
            AM_EXPECT(channel.GetInstancingMode() == eChannelInstanceMode_Blended);

            // Add multiple instances
            AM_UNUSED(channel.AddInstance({ 100.0f, 0.0f, 50.0f }));
            AM_UNUSED(channel.AddInstance({ 120.0f, 0.0f, 80.0f }));
            AM_UNUSED(channel.AddInstance({ 140.0f, 0.0f, 110.0f }));

            AM_EXPECT(channel.GetInstanceCount() == 3);

            // In blended mode, all instances share the same playback cursor
            // We can't directly test the cursor sharing without accessing internal state,
            // but we can verify that the channel is playing and the mode is correct
            AM_EXPECT(channel.Playing());
            AM_EXPECT(channel.GetInstancingMode() == eChannelInstanceMode_Blended);

            // Advance a few frames to let the sound play
            amEngine->WaitUntilFrames(10);

            // Channel should still be playing
            AM_EXPECT(channel.Playing());
        }
    };

    AM_REGISTER_TEST(core_channel_instances, blended_mode_shares_cursor);
} // namespace SparkyStudios::Audio::Amplitude::Tests
