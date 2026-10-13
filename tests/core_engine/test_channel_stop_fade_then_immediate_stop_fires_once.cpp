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

    // Stop(fade) followed by an immediate Stop(0) must settle the channel and fire eChannelEvent_Stop exactly once:
    // the immediate halt must not leave the original fade's stale voice event free to fire a second Stop.
    AM_TEST_CASE(EngineTestCase, core_engine, channel_stop_fade_then_immediate_stop_fires_once)
    {
    public:
        void Run() override
        {
            std::atomic<int> stops{ 0 };
            Channel channel = amEngine->Play(amEngine->GetSoundHandle("test_sound_01"));
            channel.On(eChannelEvent_Stop, &CountEvent, &stops);
            AM_EXPECT(WaitUntil([&]() { return channel.Playing(); }));

            channel.Stop(200.0);
            AM_EXPECT(channel.GetState()->GetChannelState() == eChannelPlaybackState_FadingOut);

            // Halt immediately, well before the 200 ms fade would have completed on its own.
            channel.Stop(0.0);

            AM_EXPECT(WaitUntil([&]() { return !channel.Valid() || channel.GetState()->Stopped(); }));

            // Give a stale event from the original fade (had it not been superseded) time to arrive too.
            amEngine->WaitUntilFrames(30);

            AM_EXPECT_EQ(1, stops.load());
        }
    };

    AM_REGISTER_TEST(core_engine, channel_stop_fade_then_immediate_stop_fires_once);
} // namespace SparkyStudios::Audio::Amplitude::Tests
