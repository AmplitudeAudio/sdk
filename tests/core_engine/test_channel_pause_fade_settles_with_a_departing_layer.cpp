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
#include <vector>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Core/Engine.h>
#include <Core/EngineInternalState.h>
#include <Core/Playback/ChannelInternalState.h>
#include <Mixer/Amplimix.h>
#include <Mixer/RealChannel.h>
#include <Sound/Sound.h>

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

    // A multi-layer channel (the shape a switch-container crossfade produces) must settle to Paused once every
    // remaining layer agrees, even while one layer is still winding down toward its own stop (the way an outgoing
    // switch-container item does): that departing layer must not flip the channel to Stopped and orphan the
    // genuinely-pausing layer.
    AM_TEST_CASE(EngineTestCase, core_engine, channel_pause_fade_settles_with_a_departing_layer)
    {
    public:
        void Run() override
        {
            std::atomic<int> stops{ 0 };

            Channel channel = amEngine->Play(amEngine->GetSoundHandle("test_sound_01"));
            channel.On(eChannelEvent_Stop, &CountEvent, &stops);
            AM_EXPECT(WaitUntil([&]() { return channel.Playing(); }));

            ChannelInternalState* state = channel.GetState();
            RealChannel& realChannel = state->GetRealChannel();
            AM_EXPECT_EQ(1ULL, realChannel.GetMixerLayerIds().size());

            // Add a second layer directly, the shape a switch-container crossfade produces on one channel.
            auto* sound = static_cast<SoundImpl*>(amEngine->GetSoundHandle("test_sound_01"));
            AM_EXPECT(realChannel.Play(std::vector<SoundInstance*>{ sound->CreateInstance() }));
            AM_EXPECT_EQ(2ULL, realChannel.GetMixerLayerIds().size());

            // Layer 0 (the original layer) departs like an outgoing switch-container item: fading out to a stop.
            // Its fade is deliberately long so it is still alive (not yet forgotten) when layer 1 below pauses.
            realChannel.FadeOutLayer(0, 200.0);

            // Pause the whole channel with a much shorter fade; only layer 1 will actually pause, while layer 0
            // is still winding down toward its own stop.
            channel.Pause(20.0);

            // The channel settles to Paused as soon as layer 1 (the only layer actually pausing) does, without
            // waiting on layer 0's own, much longer, stop fade.
            AM_EXPECT(WaitUntil([&]() { return state->Paused(); }));
            AM_EXPECT_EQ(0, stops.load());

            // The departing layer must still finish and be forgotten on its own, not leaked as a live-but-forgotten
            // voice, and the channel must stay correctly Paused (not flip to Stopped) while that happens.
            AM_EXPECT(WaitUntil([&]() { return realChannel.GetMixerLayerIds().size() == 1; }));
            AM_EXPECT(state->Paused());
            AM_EXPECT_EQ(0, stops.load());
        }
    };

    AM_REGISTER_TEST(core_engine, channel_pause_fade_settles_with_a_departing_layer);
} // namespace SparkyStudios::Audio::Amplitude::Tests
