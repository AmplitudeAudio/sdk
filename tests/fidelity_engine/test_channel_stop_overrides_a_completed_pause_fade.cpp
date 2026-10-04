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

#include <memory>
#include <string>
#include <vector>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Core/Engine.h>
#include <Core/EngineInternalState.h>
#include <Core/Playback/ChannelInternalState.h>
#include <Mixer/Amplimix.h>
#include <Mixer/RealChannel.h>

#include <Fidelity/AssetGenerator.h>
#include <Fidelity/RenderSession.h>
#include <Fidelity/Stimuli.h>

#include "PureUnitTestCase.h"
#include "TestRegistry.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    using namespace SparkyStudios::Audio::Amplitude::Fidelity;

    namespace
    {
        struct StopOverrideRaceProbe
        {
            Channel channel;
            ChannelInternalState* state = nullptr;
            AmUInt32 channelId = 0;
            AmUInt32 layer = 0;
            bool paused = false;
            bool overridden = false;
            int stops = 0;
            int stopsAtEnd = -1;
            bool endedStopped = false;
        };

        void CountStop(ChannelEventInfo info)
        {
            ++static_cast<StopOverrideRaceProbe*>(info.m_userData)->stops;
        }
    } // namespace

    // A Pause(fade) completing in the voice posts a FadedOut{Paused} event. If the game overrides it with a
    // Stop(fade) before that event is dispatched, the voice is genuinely Paused by then: Voice::Apply's Stop
    // case finishes a Paused voice immediately, without ever posting FadedOut{Stopped}. The channel must still
    // settle to Stopped and fire exactly one Stop event (via UpdateState()'s SettleStopped() backstop), not get
    // stuck FadingOut forever with _stopEventPending never consumed.
    AM_TEST_CASE(PureUnitTestCase, fidelity_engine, channel_stop_overrides_a_completed_pause_fade)
    {
    public:
        void Run() override
        {
            constexpr std::uint64_t kPlayAt = 4800;
            constexpr std::uint64_t kFrameSamples = 800;
            constexpr std::uint64_t kSettleSamples = 24000;

            const std::string sound = SoundName(*FindStimulus("sine_997_44100"), false);

            RenderSettings settings;
            settings.configFile = ConfigName("isolated", 1024, 48000, ".config.amconfig");
            settings.durationSamples = kPlayAt + 48000 + kSettleSamples;

            auto probe = std::make_shared<StopOverrideRaceProbe>();
            std::vector<TimedAction> actions{ { kPlayAt, "play",
                                                [probe, sound]()
                                                {
                                                    probe->channel = amEngine->Play(amEngine->GetSoundHandle(sound));
                                                    probe->state = probe->channel.GetState();
                                                    probe->channel.On(eChannelEvent_Stop, &CountStop, probe.get());
                                                } } };

            for (std::uint64_t at = kPlayAt + kFrameSamples; at < settings.durationSamples; at += kFrameSamples)
            {
                actions.push_back({ at, "watch",
                                    [probe]()
                                    {
                                        const AmplimixImpl& mixer = amEngine->GetState()->mixer;

                                        if (!probe->paused)
                                        {
                                            const std::vector<AmUInt32> layers = probe->state->GetRealChannel().GetMixerLayerIds();
                                            if (layers.size() != 1 || !probe->channel.Playing())
                                                return;

                                            probe->channelId = probe->state->GetRealChannel().GetID();
                                            probe->layer = layers.front();

                                            // A real, timed pause fade: the guard that lets a stop override it
                                            // only applies while the channel is genuinely FadingOut toward Paused.
                                            probe->channel.Pause(50.0);
                                            probe->paused = true;
                                            return;
                                        }

                                        if (!probe->overridden)
                                        {
                                            if (mixer.GetVoiceState(probe->channelId, probe->layer) != eVoiceState::Paused)
                                                return;

                                            // The voice just posted FadedOut{Paused}; this frame has not
                                            // dispatched it yet. Overriding right now races the voice's
                                            // already-Paused internal state against the stale event.
                                            probe->channel.Stop(200.0);
                                            probe->overridden = true;
                                            return;
                                        }

                                        probe->stopsAtEnd = probe->stops;
                                        probe->endedStopped = !probe->channel.Valid() || probe->channel.GetState()->Stopped();
                                    } });
            }

            const RenderOutcome outcome = Render(kDefaultAssetsPath, settings, std::move(actions));
            AM_EXPECT(outcome.error.empty());

            // The race was reached.
            AM_EXPECT(probe->paused);
            AM_EXPECT(probe->overridden);

            // The channel settles to Stopped with exactly one Stop event, never stuck FadingOut forever.
            AM_EXPECT(probe->endedStopped);
            AM_EXPECT_EQ(1, probe->stopsAtEnd);
        }
    };

    AM_REGISTER_TEST(fidelity_engine, channel_stop_overrides_a_completed_pause_fade);
} // namespace SparkyStudios::Audio::Amplitude::Tests
