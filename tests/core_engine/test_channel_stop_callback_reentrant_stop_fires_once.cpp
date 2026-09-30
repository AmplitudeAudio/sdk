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
        struct ReentrantStopProbe
        {
            Channel channel;
            std::atomic<int> stops{ 0 };
        };

        // The Stop callback itself calls Stop(0): the channel is still technically FadingOut (its layer is not
        // forgotten yet), so this reaches HaltInternal() again, which must not fire a second Stop event.
        void StopThenHaltAgain(ChannelEventInfo info)
        {
            auto* probe = static_cast<ReentrantStopProbe*>(info.m_userData);
            ++probe->stops;
            probe->channel.Stop(0.0);
        }
    } // namespace

    AM_TEST_CASE(EngineTestCase, core_engine, channel_stop_callback_reentrant_stop_fires_once)
    {
    public:
        void Run() override
        {
            ReentrantStopProbe probe;
            probe.channel = amEngine->Play(amEngine->GetSoundHandle("test_sound_01"));
            probe.channel.On(eChannelEvent_Stop, &StopThenHaltAgain, &probe);
            AM_EXPECT(WaitUntil([&]() { return probe.channel.Playing(); }));

            probe.channel.Stop(200.0);
            AM_EXPECT(probe.channel.GetState()->GetChannelState() == eChannelPlaybackState_FadingOut);

            AM_EXPECT(WaitUntil([&]() { return !probe.channel.Valid() || probe.channel.GetState()->Stopped(); }));

            // Give any further stale event a chance to arrive too.
            amEngine->WaitUntilFrames(30);

            AM_EXPECT_EQ(1, probe.stops.load());
        }
    };

    AM_REGISTER_TEST(core_engine, channel_stop_callback_reentrant_stop_fires_once);
} // namespace SparkyStudios::Audio::Amplitude::Tests
