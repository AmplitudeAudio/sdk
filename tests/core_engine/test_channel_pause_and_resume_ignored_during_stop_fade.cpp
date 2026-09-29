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

#include <Core/Engine.h>
#include <Core/EngineInternalState.h>
#include <Core/Playback/ChannelInternalState.h>
#include <Mixer/Amplimix.h>
#include <Mixer/RealChannel.h>

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

    // Pause() and Resume() (immediate, no-fade variants) must do nothing while a channel is fading out toward
    // Stopped: the voice itself ignores a Pause command once it is already fading toward a stop, so the game side
    // must not say Paused or Playing on a channel that is dying.
    AM_TEST_CASE(EngineTestCase, core_engine, channel_pause_and_resume_ignored_during_stop_fade)
    {
    public:
        void Run() override
        {
            std::atomic<int> pausesA{ 0 };
            std::atomic<int> stopsA{ 0 };
            std::atomic<int> resumesB{ 0 };
            std::atomic<int> stopsB{ 0 };

            Channel a = amEngine->Play(amEngine->GetSoundHandle("test_sound_01"));
            a.On(eChannelEvent_Pause, &CountEvent, &pausesA);
            a.On(eChannelEvent_Stop, &CountEvent, &stopsA);
            AM_EXPECT(WaitUntil([&]() { return a.Playing(); }));

            Channel b = amEngine->Play(amEngine->GetSoundHandle("test_sound_01"));
            b.On(eChannelEvent_Resume, &CountEvent, &resumesB);
            b.On(eChannelEvent_Stop, &CountEvent, &stopsB);
            AM_EXPECT(WaitUntil([&]() { return b.Playing(); }));

            a.Stop(200.0);
            b.Stop(200.0);
            AM_EXPECT(a.GetState()->GetChannelState() == eChannelPlaybackState_FadingOut);
            AM_EXPECT(b.GetState()->GetChannelState() == eChannelPlaybackState_FadingOut);

            // Neither of these must interrupt the stop fade in progress.
            a.Pause(0.0);
            b.Resume(0.0);

            AM_EXPECT(WaitUntil([&]() { return !a.Valid() || a.GetState()->Stopped(); }));
            AM_EXPECT(WaitUntil([&]() { return !b.Valid() || b.GetState()->Stopped(); }));

            amEngine->WaitUntilFrames(10);

            AM_EXPECT_EQ(0, pausesA.load());
            AM_EXPECT_EQ(1, stopsA.load());
            AM_EXPECT_EQ(0, resumesB.load());
            AM_EXPECT_EQ(1, stopsB.load());
        }
    };

    AM_REGISTER_TEST(core_engine, channel_pause_and_resume_ignored_during_stop_fade);
} // namespace SparkyStudios::Audio::Amplitude::Tests
