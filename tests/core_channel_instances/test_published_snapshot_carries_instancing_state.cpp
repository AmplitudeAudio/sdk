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

#include <Core/Playback/ChannelInternalState.h>

#include "EngineTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    // The audio thread learns that a channel is instanced, and in which mode, from the published snapshot alone.
    // Every change of the instancing state must therefore publish.
    AM_TEST_CASE(EngineTestCase, core_channel_instances, published_snapshot_carries_instancing_state)
    {
    public:
        void Run() override
        {
            SoundHandle sound = amEngine->GetSoundHandle("test_sound_01");
            Channel channel = amEngine->Play(sound);
            amEngine->WaitUntilFrames(2);
            AM_EXPECT(channel.Valid());

            ChannelInternalState* state = channel.GetState();

            AM_EXPECT_NOT(state->GetPublishedInstancingEnabled());
            AM_EXPECT(state->GetPublishedInstanceSnapshot().empty());

            channel.EnableInstancing(eChannelInstanceMode_Separate);
            AM_EXPECT(state->GetPublishedInstancingEnabled());
            AM_EXPECT(state->GetPublishedInstancingMode() == eChannelInstanceMode_Separate);

            AM_UNUSED(channel.AddInstance({ 1.0f, 0.0f, 0.0f }));
            AM_UNUSED(channel.AddInstance({ 2.0f, 0.0f, 0.0f }));
            AM_EXPECT_EQ(state->GetPublishedInstanceSnapshot().size(), 2u);
            AM_EXPECT_EQ(state->GetInstanceCount(), 2u);

            // Disabling clears the instances and the flag in the same published snapshot.
            channel.DisableInstancing();
            AM_EXPECT_NOT(state->GetPublishedInstancingEnabled());
            AM_EXPECT(state->GetPublishedInstancingMode() == eChannelInstanceMode_Blended);
            AM_EXPECT(state->GetPublishedInstanceSnapshot().empty());
            AM_EXPECT_EQ(state->GetInstanceCount(), 0u);

            // And enabling again starts from scratch, in the requested mode.
            channel.EnableInstancing(eChannelInstanceMode_Blended);
            AM_EXPECT(state->GetPublishedInstancingEnabled());
            AM_EXPECT(state->GetPublishedInstancingMode() == eChannelInstanceMode_Blended);

            channel.Stop(0.0);
        }
    };

    AM_REGISTER_TEST(core_channel_instances, published_snapshot_carries_instancing_state);
} // namespace SparkyStudios::Audio::Amplitude::Tests
