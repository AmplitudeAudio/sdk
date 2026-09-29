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

    AM_TEST_CASE(EngineTestCase, core_engine, voice_events_reach_the_game_thread)
    {
    public:
        void Run() override
        {
            std::atomic<int> begins{ 0 };
            std::atomic<int> stops{ 0 };

            Channel channel = amEngine->Play(amEngine->GetSoundHandle("test_sound_01"));
            AM_EXPECT(channel.Valid());
            channel.On(eChannelEvent_Begin, &CountEvent, &begins);
            channel.On(eChannelEvent_Stop, &CountEvent, &stops);

            AM_EXPECT(WaitUntil(
                [&]()
                {
                    return begins.load() == 1;
                }));

            const RealChannel& realChannel = channel.GetState()->GetRealChannel();
            const std::vector<AmUInt32> layers = realChannel.GetMixerLayerIds();
            const AmUInt32 id = realChannel.GetID();
            AM_EXPECT_EQ(1ULL, layers.size());

            channel.Stop(0.0);
            AM_EXPECT(WaitUntil(
                [&]()
                {
                    return stops.load() == 1;
                }));

            // The voice de-clicks, finishes, and the game thread releases its mixer layer.
            const AmplimixImpl& mixer = amEngine->GetState()->mixer;
            AM_EXPECT(WaitUntil(
                [&]()
                {
                    return mixer.GetVoiceState(id, layers.front()) == eVoiceState::Idle;
                }));
        }
    };

    AM_REGISTER_TEST(core_engine, voice_events_reach_the_game_thread);
} // namespace SparkyStudios::Audio::Amplitude::Tests
