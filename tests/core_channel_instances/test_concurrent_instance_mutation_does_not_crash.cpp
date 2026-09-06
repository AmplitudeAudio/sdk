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
    // Best-effort regression guard for the game-thread/audio-thread instance
    // container race: mutations from this thread race the mixer's per-callback
    // snapshot reads. Success = no crash, no deadlock, channel stays usable.
    AM_TEST_CASE(EngineTestCase, core_channel_instances, concurrent_instance_mutation_does_not_crash)
    {
    public:
        void Run() override
        {
            SoundHandle test_sound_01 = amEngine->GetSoundHandle("test_sound_01");

            Channel channel = amEngine->Play(test_sound_01);
            amEngine->WaitUntilFrames(2);
            AM_EXPECT(channel.Valid());

            channel.EnableInstancing(eChannelInstanceMode_Blended);

            for (AmUInt32 i = 0; i < 200; ++i)
            {
                if (channel.GetPlaybackState() != eChannelPlaybackState_Playing)
                    break;

                ChannelInstance instance = channel.AddInstance({ 100.0f + static_cast<AmReal32>(i), 0.0f, 50.0f });
                if (!instance.Valid())
                    continue;

                instance.SetLocation({ 110.0f + static_cast<AmReal32>(i), 1.0f, 55.0f });
                instance.SetWeight(0.5f);

                amEngine->WaitUntilNextFrame();

                channel.RemoveInstance(instance.GetId());
            }

            AM_EXPECT(channel.Valid());

            // Separate mode additionally exercises the cursor write-back slots.
            channel.ClearInstances();
            channel.DisableInstancing();
            channel.EnableInstancing(eChannelInstanceMode_Separate);

            for (AmUInt32 i = 0; i < 100; ++i)
            {
                if (channel.GetPlaybackState() != eChannelPlaybackState_Playing)
                    break;

                ChannelInstance instance = channel.AddInstance({ 50.0f, 0.0f, 25.0f });
                if (!instance.Valid())
                    continue;

                amEngine->WaitUntilNextFrame();

                channel.RemoveInstance(instance.GetId());
            }

            AM_EXPECT(channel.Valid());
        }
    };

    AM_REGISTER_TEST(core_channel_instances, concurrent_instance_mutation_does_not_crash);
} // namespace SparkyStudios::Audio::Amplitude::Tests
