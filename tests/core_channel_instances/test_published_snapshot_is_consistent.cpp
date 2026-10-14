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
    // The flag, the mode, the cursor slots and the instances come from the same publish. Reading them one by one can
    // mix two publishes; the copy cannot.
    AM_TEST_CASE(EngineTestCase, core_channel_instances, published_snapshot_is_consistent)
    {
    public:
        void Run() override
        {
            SoundHandle sound = amEngine->GetSoundHandle("test_sound_01");
            Channel channel = amEngine->Play(sound);
            amEngine->WaitUntilFrames(2);
            AM_EXPECT(channel.Valid());

            ChannelInternalState* state = channel.GetState();

            ChannelInstanceSnapshot snapshot = state->GetPublishedSnapshot();
            AM_EXPECT_NOT(snapshot.instancingEnabled);
            AM_EXPECT(snapshot.instances.empty());
            AM_EXPECT(snapshot.cursorSlots == nullptr);

            channel.EnableInstancing(eChannelInstanceMode_Separate);
            AM_UNUSED(channel.AddInstance({ 1.0f, 0.0f, 0.0f }));
            AM_UNUSED(channel.AddInstance({ 2.0f, 0.0f, 0.0f }));

            snapshot = state->GetPublishedSnapshot();
            AM_EXPECT(snapshot.instancingEnabled);
            AM_EXPECT(snapshot.instancingMode == eChannelInstanceMode_Separate);
            AM_EXPECT_EQ(snapshot.instances.size(), 2u);
            AM_EXPECT(snapshot.cursorSlots != nullptr);

            // The instances are published in ascending id order: the audio thread merges on it.
            AM_EXPECT(snapshot.instances[0].instanceId < snapshot.instances[1].instanceId);

            // A copy: later publishes do not change it.
            channel.DisableInstancing();
            AM_EXPECT(snapshot.instancingEnabled);
            AM_EXPECT_EQ(snapshot.instances.size(), 2u);
            AM_EXPECT_NOT(state->GetPublishedSnapshot().instancingEnabled);

            channel.Stop(0.0);
        }
    };

    AM_REGISTER_TEST(core_channel_instances, published_snapshot_is_consistent);
} // namespace SparkyStudios::Audio::Amplitude::Tests
