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
    AM_TEST_CASE(EngineTestCase, core_channel_instances, can_switch_between_modes)
    {
    public:
        void Run() override
        {
            SoundHandle test_sound_01 = amEngine->GetSoundHandle("test_sound_01");

            Channel channel = amEngine->Play(test_sound_01);
            amEngine->WaitUntilFrames(2);

            AM_EXPECT(channel.Valid());

            // Start with blended mode
            channel.EnableInstancing(eChannelInstanceMode_Blended);
            AM_EXPECT(channel.GetInstancingMode() == eChannelInstanceMode_Blended);

            // Add instances
            AM_UNUSED(channel.AddInstance({ 100.0f, 0.0f, 50.0f }));
            AM_UNUSED(channel.AddInstance({ 120.0f, 0.0f, 80.0f }));
            AM_EXPECT(channel.GetInstanceCount() == 2);

            // Switch to separate mode (note: this should clear existing instances based on implementation)
            channel.DisableInstancing();
            channel.EnableInstancing(eChannelInstanceMode_Separate);
            AM_EXPECT(channel.GetInstancingMode() == eChannelInstanceMode_Separate);

            // Add instances in separate mode
            AM_UNUSED(channel.AddInstance({ 100.0f, 0.0f, 50.0f }));
            AM_UNUSED(channel.AddInstance({ 120.0f, 0.0f, 80.0f }));
            AM_UNUSED(channel.AddInstance({ 140.0f, 0.0f, 110.0f }));
            AM_EXPECT(channel.GetInstanceCount() == 3);

            // Switch back to blended mode
            channel.DisableInstancing();
            channel.EnableInstancing(eChannelInstanceMode_Blended);
            AM_EXPECT(channel.GetInstancingMode() == eChannelInstanceMode_Blended);

            // Verify channel is still playing
            AM_EXPECT(channel.Playing());
        }
    };

    AM_REGISTER_TEST(core_channel_instances, can_switch_between_modes);
} // namespace SparkyStudios::Audio::Amplitude::Tests
