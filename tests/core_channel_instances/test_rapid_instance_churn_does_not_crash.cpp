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

#include <vector>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include "EngineTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    // Instances are added, moved and removed as fast as the game thread can, without waiting for frames, while the
    // mixer renders the channel in both modes. Every add and remove publishes: against the old double-buffered
    // snapshot, two publishes inside one mix block freed the buffer the mixer was copying, and the mixer walked the
    // game-owned instance list to count instances.
    AM_TEST_CASE(EngineTestCase, core_channel_instances, rapid_instance_churn_does_not_crash)
    {
    public:
        void Run() override
        {
            SoundHandle sound = amEngine->GetSoundHandle("test_sound_01");
            Channel channel = amEngine->Play(sound);
            amEngine->WaitUntilFrames(2);
            AM_EXPECT(channel.Valid());

            for (const eChannelInstanceMode mode : { eChannelInstanceMode_Blended, eChannelInstanceMode_Separate })
            {
                channel.EnableInstancing(mode);

                for (AmUInt32 round = 0; round < 1500; ++round)
                {
                    std::vector<ChannelInstance> instances;
                    for (AmUInt32 i = 0; i < 4; ++i)
                        instances.push_back(channel.AddInstance({ static_cast<AmReal32>(round % 50), static_cast<AmReal32>(i), 0.0f }));

                    for (const auto& instance : instances)
                    {
                        if (!instance.Valid())
                            continue;

                        instance.SetLocation({ 1.0f, static_cast<AmReal32>(round % 7), 0.0f });
                        instance.SetWeight(0.5f);
                    }

                    for (const auto& instance : instances)
                        if (instance.Valid())
                            channel.RemoveInstance(instance.GetId());
                }

                AM_EXPECT_EQ(channel.GetInstanceCount(), 0u);
                AM_EXPECT(channel.Valid());

                channel.DisableInstancing();
            }

            channel.Stop(0.0);
        }
    };

    AM_REGISTER_TEST(core_channel_instances, rapid_instance_churn_does_not_crash);
} // namespace SparkyStudios::Audio::Amplitude::Tests
