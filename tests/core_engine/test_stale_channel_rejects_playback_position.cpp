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
    AM_TEST_CASE(EngineTestCase, core_engine, stale_channel_rejects_playback_position)
    {
    public:
        void Run() override
        {
            SoundHandle sound = amEngine->GetSoundHandle("test_sound_01");

            Channel channel = amEngine->Play(sound);
            amEngine->WaitUntilFrames(2);

            AM_EXPECT(channel.Valid());
            AM_EXPECT(channel.Playing());

            ChannelInternalState* state = channel.GetState();
            const AmUInt64 validStateId = state->GetChannelStateId();

            state->SetChannelStateId(validStateId + 1);
            AM_EXPECT_NOT(channel.SetPlaybackPosition(10.0));
            AM_EXPECT_EQ(channel.GetPlaybackPosition(), 0.0);

            state->SetChannelStateId(validStateId);
            channel.Stop(0);
        }
    };

    AM_REGISTER_TEST(core_engine, stale_channel_rejects_playback_position);
} // namespace SparkyStudios::Audio::Amplitude::Tests
