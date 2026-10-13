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
    // A burst that fills the engine's next-frame queue must not lose a channel's Stop (the channel would never be
    // recycled): the event waits on the channel and fires once the queue drains.
    AM_TEST_CASE(EngineTestCase, core_engine, channel_events_survive_a_full_callback_queue)
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

            ChannelInternalState* state = channel.GetState();

            // Hold the frame loop so nothing drains the queue while it is filled.
            amEngine->Pause(true);
            Thread::Sleep(100);

            AmSize filled = 0;
            while (amEngine->OnNextFrame([](AmTime) {}))
                ++filled;

            AM_EXPECT(filled > 0);

            channel.Stop(0.0);
            AM_EXPECT(state->Stopped());
            AM_EXPECT(state->HasDeferredEvents());

            amEngine->Pause(false);

            AM_EXPECT(WaitUntil(
                [&]()
                {
                    return counter->stopped.load() > 0;
                }));

            amEngine->WaitUntilFrames(10);
            AM_EXPECT_EQ(1, counter->stopped.load());
            AM_EXPECT(WaitUntil(
                [&]()
                {
                    return !state->IsAlive();
                }));
        }
    };

    AM_REGISTER_TEST(core_engine, channel_events_survive_a_full_callback_queue);
} // namespace SparkyStudios::Audio::Amplitude::Tests
