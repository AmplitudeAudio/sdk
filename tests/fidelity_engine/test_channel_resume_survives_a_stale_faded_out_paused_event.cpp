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
        struct ResumeRaceProbe
        {
            Channel channel;
            ChannelInternalState* state = nullptr;
            AmUInt32 channelId = 0;
            AmUInt32 layer = 0;
            bool paused = false;
            bool resumed = false;
            int stops = 0;
            bool endedPlaying = false;
            AmSize finalLayers = 0;
            int stopsAtEnd = -1;
        };

        void CountStop(ChannelEventInfo info)
        {
            ++static_cast<ResumeRaceProbe*>(info.m_userData)->stops;
        }
    } // namespace

    // A Pause(0)/pause fade completing in the voice posts a FadedOut{Paused} event; if the game calls Resume()
    // before the engine dispatches that event, the stale event must not re-mark the (now genuinely playing again)
    // layer as paused, which would make RealChannel::Playing() lie and UpdateState() drop the still-sounding voice.
    AM_TEST_CASE(PureUnitTestCase, fidelity_engine, channel_resume_survives_a_stale_faded_out_paused_event)
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

            auto probe = std::make_shared<ResumeRaceProbe>();
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

                                            // Immediate pause: settles the game-side state at once, the voice still
                                            // renders its de-click and posts FadedOut{Paused} once that completes.
                                            probe->channel.Pause(0.0);
                                            probe->paused = true;
                                            return;
                                        }

                                        if (!probe->resumed)
                                        {
                                            if (mixer.GetVoiceState(probe->channelId, probe->layer) != eVoiceState::Paused)
                                                return;

                                            // The voice just posted FadedOut{Paused}; this frame has not dispatched
                                            // it yet (DispatchVoiceEvents runs inside the AdvanceFrame this action
                                            // precedes). Resuming right now races the stale event.
                                            probe->channel.Resume(0.0);
                                            probe->resumed = true;
                                            return;
                                        }

                                        probe->endedPlaying = probe->channel.Playing();
                                        probe->finalLayers = probe->state->GetRealChannel().GetMixerLayerIds().size();
                                        probe->stopsAtEnd = probe->stops;
                                    } });
            }

            const RenderOutcome outcome = Render(kDefaultAssetsPath, settings, std::move(actions));
            AM_EXPECT(outcome.error.empty());

            // The race was reached.
            AM_EXPECT(probe->paused);
            AM_EXPECT(probe->resumed);

            // The stale FadedOut{Paused} must not orphan the still-playing voice.
            AM_EXPECT(probe->endedPlaying);
            AM_EXPECT_EQ(1ULL, probe->finalLayers);
            AM_EXPECT_EQ(0, probe->stopsAtEnd);
        }
    };

    AM_REGISTER_TEST(fidelity_engine, channel_resume_survives_a_stale_faded_out_paused_event);
} // namespace SparkyStudios::Audio::Amplitude::Tests
