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
    // A Stop scheduled for a later frame leaves the channel as it is until that frame: it keeps following its gains, and a
    // Stop issued meanwhile wins.
    AM_TEST_CASE(EngineTestCase, core_engine, channel_scheduled_stop_stays_pending)
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
            const AmUInt64 target = amEngine->GetAudioClock() + amEngine->GetAudioClockRate() * 2; // two seconds ahead
            channel.Stop(100.0, target);

            // Still playing: nothing fades yet.
            AM_EXPECT(state->GetChannelState() == eChannelPlaybackState_Playing);

            // Gain updates still reach the voice layer. The frame loop is held so it cannot overwrite the gain.
            amEngine->Pause(true);
            Thread::Sleep(100);
            AM_EXPECT(state->GetRealChannel().GetGain(kAmInvalidObjectId) != 0.123f);
            state->SetGain(0.123f);
            AM_EXPECT_EQ(0.123f, state->GetRealChannel().GetGain(kAmInvalidObjectId));
            amEngine->Pause(false);

            // A Stop now overrides the scheduled one: the channel stops within the fade, not at the scheduled frame.
            channel.Stop();
            AM_EXPECT(WaitUntil(
                [&]()
                {
                    return state->Stopped();
                },
                20));
            AM_EXPECT(amEngine->GetAudioClock() < target);

            AM_EXPECT(WaitUntil(
                [&]()
                {
                    return counter->stopped.load() > 0;
                }));

            // Past the scheduled frame the stale command does nothing more: still exactly one Stop.
            AM_EXPECT(WaitUntil(
                [&]()
                {
                    return amEngine->GetAudioClock() > target;
                }));
            amEngine->WaitUntilFrames(10);
            AM_EXPECT_EQ(1, counter->stopped.load());
        }
    };

    AM_REGISTER_TEST(core_engine, channel_scheduled_stop_stays_pending);
} // namespace SparkyStudios::Audio::Amplitude::Tests
