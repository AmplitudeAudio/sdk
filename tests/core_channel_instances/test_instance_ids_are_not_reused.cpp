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
    // The audio thread matches cursors by instance id: an id must not name two instances, whatever happens to
    // the instancing state in between.
    AM_TEST_CASE(EngineTestCase, core_channel_instances, instance_ids_are_not_reused)
    {
    public:
        void Run() override
        {
            SoundHandle sound = amEngine->GetSoundHandle("test_sound_01");
            Channel channel = amEngine->Play(sound);
            amEngine->WaitUntilFrames(2);
            AM_EXPECT(channel.Valid());

            channel.Pause();

            channel.EnableInstancing(eChannelInstanceMode_Separate);
            ChannelInstance first = channel.AddInstance({ 1.0f, 0.0f, 0.0f });
            AM_EXPECT(first.Valid());

            channel.DisableInstancing();
            AM_EXPECT_NOT(first.Valid());

            channel.EnableInstancing(eChannelInstanceMode_Separate);
            ChannelInstance second = channel.AddInstance({ 1.0f, 0.0f, 0.0f });
            AM_EXPECT(second.Valid());
            AM_EXPECT(second.GetId() != first.GetId());

            channel.ClearInstances();

            ChannelInstance third = channel.AddInstance({ 1.0f, 0.0f, 0.0f });
            AM_EXPECT(third.Valid());
            AM_EXPECT(third.GetId() != first.GetId());
            AM_EXPECT(third.GetId() != second.GetId());

            channel.Resume();
            channel.Stop(0.0);
        }
    };

    AM_REGISTER_TEST(core_channel_instances, instance_ids_are_not_reused);
} // namespace SparkyStudios::Audio::Amplitude::Tests
