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

    AM_TEST_CASE(EngineTestCase, core_engine, channel_stop_fade_ends_on_voice_event)
    {
    public:
        void Run() override
        {
            std::atomic<int> stops{ 0 };
            Channel channel = amEngine->Play(amEngine->GetSoundHandle("test_sound_01"));
            channel.On(eChannelEvent_Stop, &CountEvent, &stops);
            AM_EXPECT(WaitUntil([&]() { return channel.Playing(); }));

            // 200 ms is about 12 frames at 60 fps: the channel stays fading for most of them.
            channel.Stop(200.0);
            AM_EXPECT(channel.GetState()->GetChannelState() == eChannelPlaybackState_FadingOut);
            amEngine->WaitUntilFrames(3);
            AM_EXPECT(channel.GetState()->GetChannelState() == eChannelPlaybackState_FadingOut);
            AM_EXPECT_EQ(0, stops.load());

            AM_EXPECT(WaitUntil([&]() { return stops.load() == 1; }));
            AM_EXPECT(WaitUntil([&]() { return !channel.Valid() || channel.GetState()->Stopped(); }));
        }
    };

    AM_REGISTER_TEST(core_engine, channel_stop_fade_ends_on_voice_event);
} // namespace SparkyStudios::Audio::Amplitude::Tests
