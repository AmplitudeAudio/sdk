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

#include <atomic>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include "EngineTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    namespace
    {
        void CountEvent(ChannelEventInfo info)
        {
            ++*static_cast<std::atomic<int>*>(info.m_userData);
        }
    } // namespace

    AM_TEST_CASE(EngineTestCase, core_engine, schedule_start_waits_for_its_clock)
    {
    public:
        void Run() override
        {
            std::atomic<int> begins{ 0 };
            Channel channel = amEngine->Play(amEngine->GetSoundHandle("test_sound_01"));
            channel.On(eChannelEvent_Begin, &CountEvent, &begins);

            const AmUInt64 target = amEngine->GetAudioClock() + amEngine->GetAudioClockRate() * 2; // two seconds
            AM_EXPECT(channel.ScheduleStart(target));

            amEngine->WaitUntilFrames(10);

            // The null driver renders faster than real time, so a descheduled game thread can see the clock pass the
            // target: read the count first, then the clock, and only require the start to have waited when it began.
            const int seenBegins = begins.load();
            const AmUInt64 seenClock = amEngine->GetAudioClock();
            AM_EXPECT(seenBegins == 0 || seenClock >= target);

            AM_EXPECT(WaitUntil([&]() { return begins.load() == 1; }));
            AM_EXPECT(amEngine->GetAudioClock() >= target);
            channel.Stop(0.0);
        }
    };

    AM_REGISTER_TEST(core_engine, schedule_start_waits_for_its_clock);
} // namespace SparkyStudios::Audio::Amplitude::Tests
