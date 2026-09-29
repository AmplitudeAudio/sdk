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

    // A Stop(fade) issued while a Pause(fade) is still in progress must override it: the voice supports stopping
    // over a pause fade (it restarts the fade from the current gain), so the channel must end Stopped with exactly
    // one Stop event and no Pause event, not silently drop the stop request.
    AM_TEST_CASE(EngineTestCase, core_engine, channel_stop_fade_overrides_pause_fade)
    {
    public:
        void Run() override
        {
            std::atomic<int> pauses{ 0 };
            std::atomic<int> stops{ 0 };

            Channel channel = amEngine->Play(amEngine->GetSoundHandle("test_sound_01"));
            channel.On(eChannelEvent_Pause, &CountEvent, &pauses);
            channel.On(eChannelEvent_Stop, &CountEvent, &stops);
            AM_EXPECT(WaitUntil([&]() { return channel.Playing(); }));

            channel.Pause(200.0);
            AM_EXPECT(channel.GetState()->GetChannelState() == eChannelPlaybackState_FadingOut);

            // Overrides the pause fade in progress; must not be dropped.
            channel.Stop(200.0);
            AM_EXPECT(channel.GetState()->GetChannelState() == eChannelPlaybackState_FadingOut);

            AM_EXPECT(WaitUntil([&]() { return !channel.Valid() || channel.GetState()->Stopped(); }));

            amEngine->WaitUntilFrames(10);

            AM_EXPECT_EQ(0, pauses.load());
            AM_EXPECT_EQ(1, stops.load());
        }
    };

    AM_REGISTER_TEST(core_engine, channel_stop_fade_overrides_pause_fade);
} // namespace SparkyStudios::Audio::Amplitude::Tests
