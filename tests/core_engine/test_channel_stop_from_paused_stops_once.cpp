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
#include "VirtualChannelTestUtils.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    // Pause() then Stop() with the default fade (the pause menu, then leave the level) stops the channel once and
    // recycles it.
    AM_TEST_CASE(EngineTestCase, core_engine, channel_stop_from_paused_stops_once)
    {
    public:
        void Run() override
        {
            EventCounter* counter = NewCounter();
            Channel channel = amEngine->Play(amEngine->GetSoundHandle("test_sound_06"));
            channel.On(eChannelEvent_Stop, &CountStop, counter);
            AM_EXPECT(WaitUntil(
                [&]()
                {
                    return channel.Playing();
                }));

            channel.Pause(0.0);
            AM_EXPECT(channel.GetState()->Paused());

            ChannelInternalState* state = channel.GetState();
            channel.Stop();
            AM_EXPECT(state->Stopped());

            AM_EXPECT(WaitUntil(
                [&]()
                {
                    return counter->stopped.load() > 0;
                }));

            amEngine->WaitUntilFrames(10);
            AM_EXPECT_EQ(1, counter->stopped.load());

            // Recycled once its voice is gone.
            AM_EXPECT(WaitUntil(
                [&]()
                {
                    return !state->IsAlive();
                }));
        }
    };

    AM_REGISTER_TEST(core_engine, channel_stop_from_paused_stops_once);
} // namespace SparkyStudios::Audio::Amplitude::Tests
