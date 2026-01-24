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
    AM_TEST_CASE(EngineTestCase, core_channel_instances, can_add_and_remove_instances)
    {
    public:
        void Run() override
        {
            SoundHandle test_sound_01 = amEngine->GetSoundHandle("test_sound_01");

            Channel channel = amEngine->Play(test_sound_01);
            amEngine->WaitUntilFrames(2);

            AM_EXPECT(channel.Valid());

            // Enable instancing
            channel.EnableInstancing(eChannelInstanceMode_Blended);
            AM_EXPECT(channel.GetInstanceCount() == 0);

            // Add instances
            ChannelInstance instance1 = channel.AddInstance({ 100.0f, 0.0f, 50.0f });
            AM_EXPECT(instance1.Valid());
            AM_EXPECT(channel.GetInstanceCount() == 1);

            ChannelInstance instance2 = channel.AddInstance({ 120.0f, 0.0f, 80.0f });
            AM_EXPECT(instance2.Valid());
            AM_EXPECT(channel.GetInstanceCount() == 2);

            ChannelInstance instance3 = channel.AddInstance({ 140.0f, 0.0f, 110.0f });
            AM_EXPECT(instance3.Valid());
            AM_EXPECT(channel.GetInstanceCount() == 3);

            // Remove an instance
            channel.RemoveInstance(instance2.GetId());
            AM_EXPECT(channel.GetInstanceCount() == 2);

            // Try to get removed instance - should be invalid
            ChannelInstance removedInstance = channel.GetInstance(instance2.GetId());
            AM_EXPECT_NOT(removedInstance.Valid());

            // Clear all instances
            channel.ClearInstances();
            AM_EXPECT(channel.GetInstanceCount() == 0);
        }
    };

    AM_REGISTER_TEST(core_channel_instances, can_add_and_remove_instances);
} // namespace SparkyStudios::Audio::Amplitude::Tests
