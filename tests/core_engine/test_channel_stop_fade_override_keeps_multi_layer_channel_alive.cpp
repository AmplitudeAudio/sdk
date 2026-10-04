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

    // A Stop(fade) overriding a Pause(fade) on a multi-layer channel where one layer is already (game-side) marked
    // paused and the other is still fading: the channel stays FadingOut while either voice still renders its stop fade,
    // is not recycled until both are forgotten, and fires exactly one Stop event once it settles.
    AM_TEST_CASE(EngineTestCase, core_engine, channel_stop_fade_override_keeps_multi_layer_channel_alive)
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
            const AmUInt64 originalStateId = state->GetChannelStateId();

            const std::vector<AmUInt32> initial = realChannel.GetMixerLayerIds();
            AM_EXPECT_EQ(1ULL, initial.size());
            const AmUInt32 layer0MixerId = initial.front();

            // Add a second layer directly, the shape a switch-container crossfade produces on one channel.
            auto* sound = static_cast<SoundImpl*>(amEngine->GetSoundHandle("test_sound_01"));
            AM_EXPECT(realChannel.Play(std::vector<SoundInstance*>{ sound->CreateInstance() }));
            AM_EXPECT_EQ(2ULL, realChannel.GetMixerLayerIds().size());

            // Pause the whole channel with a long fade; neither layer's voice has genuinely reached Paused yet.
            channel.Pause(300.0);
            AM_EXPECT(state->GetChannelState() == eChannelPlaybackState_FadingOut);

            // Layer 0's voice reports it finished its pause fade, the way the mixer really would once it gets
            // there: marks it paused (game-side) while layer 1 is still fading.
            const AmplimixImpl& mixer = amEngine->GetState()->mixer;
            const AmUInt32 channelId = realChannel.GetID();
            state->OnVoiceFadedOut(layer0MixerId, eVoiceFadeTarget::Paused, 0, 0);

            const AmUInt32 layer1MixerId = [&]() -> AmUInt32
            {
                for (const AmUInt32 id : realChannel.GetMixerLayerIds())
                    if (id != layer0MixerId)
                        return id;
                return kAmInvalidObjectId;
            }();
            AM_EXPECT_NE(kAmInvalidObjectId, layer1MixerId);

            // Stop overrides the pause fade while layer 0 is (game-side) marked paused and layer 1 is still
            // fading: both real voices are still far from Idle at this point.
            channel.Stop(200.0);
            AM_EXPECT(state->GetChannelState() == eChannelPlaybackState_FadingOut);

            // While either voice still renders its stop fade, the channel stays FadingOut, is not recycled and has not
            // fired Stop yet.
            amEngine->WaitUntilFrames(2);
            AM_EXPECT(
                mixer.GetVoiceState(channelId, layer0MixerId) != eVoiceState::Idle ||
                mixer.GetVoiceState(channelId, layer1MixerId) != eVoiceState::Idle);
            AM_EXPECT(state->GetChannelState() == eChannelPlaybackState_FadingOut);
            AM_EXPECT_EQ(originalStateId, state->GetChannelStateId());
            AM_EXPECT_EQ(0, stops.load());

            // Both voices eventually finish and get released; only then does the channel settle Stopped, and
            // only then may it be recycled.
            AM_EXPECT(WaitUntil(
                [&]()
                {
                    return mixer.GetVoiceState(channelId, layer0MixerId) == eVoiceState::Idle &&
                        mixer.GetVoiceState(channelId, layer1MixerId) == eVoiceState::Idle;
                }));
            AM_EXPECT(WaitUntil([&]() { return state->Stopped(); }));

            amEngine->WaitUntilFrames(10);
            AM_EXPECT_EQ(1, stops.load());
        }
    };

    AM_REGISTER_TEST(core_engine, channel_stop_fade_override_keeps_multi_layer_channel_alive);
} // namespace SparkyStudios::Audio::Amplitude::Tests
