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
    AM_TEST_CASE(EngineTestCase, core_channel_instances, instance_count_is_correct)
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

            // Initial count should be 0
            AM_EXPECT(channel.GetInstanceCount() == 0);

            // Add multiple instances and verify count
            for (AmSize i = 0; i < 10; ++i)
            {
                AM_UNUSED(channel.AddInstance({ static_cast<AmReal32>(i * 10.0f), 0.0f, 0.0f }));
                AM_EXPECT(channel.GetInstanceCount() == i + 1);
            }

            // Remove instances and verify count
            AmSize currentCount = channel.GetInstanceCount();
            for (AmSize i = 0; i < 5; ++i)
            {
                ChannelInstance instance = channel.GetInstance(i + 1); // IDs start from 1
                if (instance.Valid())
                {
                    channel.RemoveInstance(instance.GetId());
                    currentCount--;
                    AM_EXPECT(channel.GetInstanceCount() == currentCount);
                }
            }

            // Clear all instances
            channel.ClearInstances();
            AM_EXPECT(channel.GetInstanceCount() == 0);
        }
    };

    AM_REGISTER_TEST(core_channel_instances, instance_count_is_correct);
} // namespace SparkyStudios::Audio::Amplitude::Tests
