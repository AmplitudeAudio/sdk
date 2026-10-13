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

#include <vector>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include "EngineTestCase.h"
#include "TestRegistry.h"
#include "VirtualChannelTestUtils.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    namespace
    {
        /// Fills the pool with looping sounds, schedules @p target for the quietest one half a second ahead, then lets a louder
        /// sound take its real channel. Returns the scheduled frame; @p low and @p high keep the channels alive.
        inline AmUInt64 ScheduleThenSteal(
            std::vector<Channel>& low, Channel& high, EventCounter* counter, eChannelPlaybackState target, AmUInt64& scheduled)
        {
            SoundHandle sound = amEngine->GetSoundHandle("test_sound_06");
            for (int i = 0; i < 50; ++i)
            {
                low.push_back(amEngine->Play(sound, AmVector3{ 0.0f, 0.0f, 0.0f }, i == 49 ? 0.4f : 0.5f));
                if (i == 49)
                    low.back().On(eChannelEvent_Stop, &CountStop, counter);
            }

            amEngine->WaitUntilFrames(2);

            scheduled = amEngine->GetAudioClock() + amEngine->GetAudioClockRate() / 2;
            if (target == eChannelPlaybackState_Stopped)
                low.back().Stop(0.0, scheduled);
            else
                low.back().Pause(0.0, scheduled);

            high = amEngine->Play(sound, AmVector3{ 0.0f, 0.0f, 0.0f }, 1.0f);
            return scheduled;
        }
    } // namespace

    // A Stop scheduled before the channel loses its real channel still fires, once, at the scheduled frame: a virtual
    // channel has nothing to fade, so it stops at once instead of staying silent and alive.
    AM_TEST_CASE(EngineTestCase, core_engine, channel_scheduled_stop_resolves_on_a_virtual_channel)
    {
    public:
        void Run() override
        {
            std::vector<Channel> low;
            Channel high;
            AmUInt64 scheduled = 0;
            EventCounter* counter = NewCounter();
            ScheduleThenSteal(low, high, counter, eChannelPlaybackState_Stopped, scheduled);

            AM_EXPECT(WaitUntil(
                [&]()
                {
                    return !low.back().GetState()->IsReal();
                }));
            AM_EXPECT_EQ(0, counter->stopped.load());

            AM_EXPECT(WaitUntil(
                [&]()
                {
                    return counter->stopped.load() > 0;
                },
                kMaxMixedWaitFrames));
            AM_EXPECT(low.back().GetState()->Stopped());
            AM_EXPECT(amEngine->GetAudioClock() < scheduled + amEngine->GetAudioClockRate() / 2);

            amEngine->WaitUntilFrames(10);
            AM_EXPECT_EQ(1, counter->stopped.load());

            for (auto& channel : low)
                channel.Stop(0.0);
            high.Stop(0.0);
        }
    };

    // The same with a scheduled Pause: the virtual channel ends Paused (like a demoted paused channel), not Stopped.
    AM_TEST_CASE(EngineTestCase, core_engine, channel_scheduled_pause_resolves_on_a_virtual_channel)
    {
    public:
        void Run() override
        {
            std::vector<Channel> low;
            Channel high;
            AmUInt64 scheduled = 0;
            EventCounter* counter = NewCounter();
            ScheduleThenSteal(low, high, counter, eChannelPlaybackState_Paused, scheduled);

            AM_EXPECT(WaitUntil(
                [&]()
                {
                    return !low.back().GetState()->IsReal();
                }));
            AM_EXPECT(WaitUntil(
                [&]()
                {
                    return amEngine->GetAudioClock() > scheduled;
                },
                kMaxMixedWaitFrames));
            amEngine->WaitUntilFrames(10);

            ChannelInternalState* state = low.back().GetState();
            AM_EXPECT(state->IsAlive());
            AM_EXPECT_NOT(state->IsReal());
            AM_EXPECT(state->Paused());
            AM_EXPECT_EQ(0, counter->stopped.load());

            low.back().Stop(0.0);
            AM_EXPECT(WaitUntil(
                [&]()
                {
                    return counter->stopped.load() > 0;
                }));
            amEngine->WaitUntilFrames(10);
            AM_EXPECT_EQ(1, counter->stopped.load());

            for (auto& channel : low)
                channel.Stop(0.0);
            high.Stop(0.0);
        }
    };

    AM_REGISTER_TEST(core_engine, channel_scheduled_stop_resolves_on_a_virtual_channel);
    AM_REGISTER_TEST(core_engine, channel_scheduled_pause_resolves_on_a_virtual_channel);
} // namespace SparkyStudios::Audio::Amplitude::Tests
