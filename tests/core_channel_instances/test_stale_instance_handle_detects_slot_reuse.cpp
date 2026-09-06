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
    AM_TEST_CASE(EngineTestCase, core_channel_instances, stale_instance_handle_detects_slot_reuse)
    {
    public:
        void Run() override
        {
            SoundHandle test_sound_01 = amEngine->GetSoundHandle("test_sound_01");

            Channel channel = amEngine->Play(test_sound_01);
            amEngine->WaitUntilFrames(2);

            AM_EXPECT(channel.Valid());

            channel.EnableInstancing(eChannelInstanceMode_Blended);

            ChannelInstance first = channel.AddInstance({ 100.0f, 0.0f, 50.0f });
            AM_EXPECT(first.Valid());
            const AmChannelInstanceID firstId = first.GetId();

            channel.RemoveInstance(firstId);

            // The memory pool recycles the freed instance state for the next add.
            ChannelInstance second = channel.AddInstance({ 200.0f, 0.0f, 80.0f });
            AM_EXPECT(second.Valid());
            AM_EXPECT(second.GetId() != firstId);

            // The stale handle must not validate against the recycled state, even
            // though the memory has been reused for a live instance.
            AM_EXPECT_NOT(first.Valid());

            // The new handle keeps working on the recycled state.
            second.SetLocation({ 0.0f, 0.0f, 0.0f });
            AM_EXPECT(channel.GetInstance(second.GetId()).GetLocation().x == 0.0f);
        }
    };

    AM_REGISTER_TEST(core_channel_instances, stale_instance_handle_detects_slot_reuse);
} // namespace SparkyStudios::Audio::Amplitude::Tests
