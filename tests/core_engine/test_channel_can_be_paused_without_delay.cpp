// Copyright (c) 2021-present Sparky Studios. All rights reserved.
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
    AM_TEST_CASE(EngineTestCase, core_engine, channel_can_be_paused_without_delay)
    {
    public:
        void Run() override
        {
            AmVector3 location = { 10.0f, 20.0f, 30.0f };
            AmReal32 userGain = 0.36f;
            Channel channel = amEngine->Play(100, location, userGain);
            amEngine->WaitUntilFrames(2); // Playing is done in the next frame

            AM_EXPECT(channel.Valid());
            AM_EXPECT(channel.Playing());

            // Test pausing without delay
            channel.Pause(0);
            AM_EXPECT_EQ(channel.GetPlaybackState(), eChannelPlaybackState_Paused);
            AM_EXPECT_NOT(channel.Playing());

            Thread::Sleep(kAmSecond); // wait for sixty frames

            // Test resuming without delay
            channel.Resume(0);
            AM_EXPECT_EQ(channel.GetPlaybackState(), eChannelPlaybackState_Playing);
            AM_EXPECT(channel.Playing());
        }
    };

    AM_REGISTER_TEST(core_engine, channel_can_be_paused_without_delay);
} // namespace SparkyStudios::Audio::Amplitude::Tests
