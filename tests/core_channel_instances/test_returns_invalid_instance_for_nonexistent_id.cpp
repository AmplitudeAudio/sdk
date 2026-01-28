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
    AM_TEST_CASE(EngineTestCase, core_channel_instances, returns_invalid_instance_for_nonexistent_id)
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

            // Try to get an instance with invalid ID
            ChannelInstance invalidInstance = channel.GetInstance(999999);
            AM_EXPECT(!invalidInstance.Valid());

            // Add an instance
            ChannelInstance validInstance = channel.AddInstance({ 100.0f, 0.0f, 50.0f });
            AM_EXPECT(validInstance.Valid());

            // Try to get the valid instance
            ChannelInstance retrievedInstance = channel.GetInstance(validInstance.GetId());
            AM_EXPECT(retrievedInstance.Valid());
            AM_EXPECT(retrievedInstance.GetId() == validInstance.GetId());

            // Remove the instance
            channel.RemoveInstance(validInstance.GetId());

            // Try to get the removed instance - should be invalid
            ChannelInstance removedInstance = channel.GetInstance(validInstance.GetId());
            AM_EXPECT(!removedInstance.Valid());
        }
    };

    AM_REGISTER_TEST(core_channel_instances, returns_invalid_instance_for_nonexistent_id);
} // namespace SparkyStudios::Audio::Amplitude::Tests
