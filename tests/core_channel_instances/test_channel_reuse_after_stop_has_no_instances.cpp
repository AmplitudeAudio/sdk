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
    AM_TEST_CASE(EngineTestCase, core_channel_instances, channel_reuse_after_stop_has_no_instances)
    {
    public:
        void Run() override
        {
            SoundHandle test_sound_01 = amEngine->GetSoundHandle("test_sound_01");

            Channel channel = amEngine->Play(test_sound_01);
            amEngine->WaitUntilFrames(2);
            AM_EXPECT(channel.Valid());

            channel.EnableInstancing(eChannelInstanceMode_Blended);
            AM_EXPECT(channel.AddInstance({ 100.0f, 0.0f, 50.0f }).Valid());
            AM_EXPECT(channel.AddInstance({ 200.0f, 0.0f, 80.0f }).Valid());
            AM_EXPECT(channel.GetInstanceCount() == 2);

            // Stop and wait for the engine to recycle the channel state.
            channel.Stop();
            amEngine->WaitUntilFrames(3);

            // Free lists are LIFO: the next play reuses the same channel state.
            Channel reused = amEngine->Play(test_sound_01);
            amEngine->WaitUntilFrames(2);
            AM_EXPECT(reused.Valid());

            // The recycled channel must not carry over instances or instancing state.
            AM_EXPECT(reused.GetInstanceCount() == 0);
            AM_EXPECT_NOT(reused.IsInstancingEnabled());
        }
    };

    AM_REGISTER_TEST(core_channel_instances, channel_reuse_after_stop_has_no_instances);
} // namespace SparkyStudios::Audio::Amplitude::Tests
