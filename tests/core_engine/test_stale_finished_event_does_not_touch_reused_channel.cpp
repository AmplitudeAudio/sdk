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
        struct ReuseProbe
        {
            Channel first;
            Channel second;
            ChannelInternalState* firstState = nullptr;
            AmUInt32 channelId = 0;
            AmUInt32 firstLayer = 0;
            bool reused = false;
            int secondBegins = 0;
            int secondEnds = 0;
            int secondStops = 0;
            int observedBegins = 0;
            int observedEnds = 0;
            int observedStops = 0;
            bool secondPlaying = false;
            AmSize secondLayers = 0;
            eVoiceState firstLayerState = eVoiceState::Scheduled;
        };

        void Count(ChannelEventInfo info)
        {
            ++*static_cast<int*>(info.m_userData);
        }
    } // namespace

    // A voice ends naturally while its channel is recycled and handed to a new sound before the voice's Ended and
    // Finished events are dispatched (the engine recycles at once when a sound is unloaded or a channel is stolen).
    // Those stale events must release the old mixer layer but never reach the new sound: no End, no Stop, no halt.
    AM_TEST_CASE(PureUnitTestCase, core_engine, stale_finished_event_does_not_touch_reused_channel)
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
            settings.durationSamples = kPlayAt + 2 * 48000 + 2 * kSettleSamples;

            auto probe = std::make_shared<ReuseProbe>();
            std::vector<TimedAction> actions{ { kPlayAt, "play",
                                                [probe, sound]()
                                                {
                                                    probe->first = amEngine->Play(amEngine->GetSoundHandle(sound));
                                                    probe->firstState = probe->first.GetState();
                                                } } };

            for (std::uint64_t at = kPlayAt + kFrameSamples; at < settings.durationSamples; at += kFrameSamples)
            {
                actions.push_back({ at, "watch",
                                    [probe, sound]()
                                    {
                                        const AmplimixImpl& mixer = amEngine->GetState()->mixer;

                                        if (!probe->reused)
                                        {
                                            const RealChannel& realChannel = probe->firstState->GetRealChannel();
                                            const std::vector<AmUInt32> layers = realChannel.GetMixerLayerIds();
                                            if (layers.size() != 1)
                                                return;

                                            probe->channelId = realChannel.GetID();
                                            probe->firstLayer = layers.front();

                                            // Wait until the source ended: its Ended (and maybe Finished) event is
                                            // published but not dispatched yet, the frame below dispatches it.
                                            const eVoiceState state = mixer.GetVoiceState(probe->channelId, probe->firstLayer);
                                            if (state != eVoiceState::Ending && state != eVoiceState::Finished)
                                                return;

                                            // Recycle the channel at once, as unloading a sound does, and reuse it.
                                            InsertIntoFreeList(amEngine->GetState(), probe->firstState);
                                            probe->second = amEngine->Play(amEngine->GetSoundHandle(sound));
                                            probe->reused = probe->second.GetState() == probe->firstState;
                                            probe->second.On(eChannelEvent_Begin, &Count, &probe->secondBegins);
                                            probe->second.On(eChannelEvent_End, &Count, &probe->secondEnds);
                                            probe->second.On(eChannelEvent_Stop, &Count, &probe->secondStops);
                                            return;
                                        }

                                        // Observed during playback: the engine teardown after the render stops every
                                        // channel and fires Stop.
                                        probe->observedBegins = probe->secondBegins;
                                        probe->observedEnds = probe->secondEnds;
                                        probe->observedStops = probe->secondStops;
                                        probe->secondPlaying = probe->second.Playing();
                                        probe->secondLayers = probe->second.GetState()->GetRealChannel().GetMixerLayerIds().size();
                                        probe->firstLayerState = mixer.GetVoiceState(probe->channelId, probe->firstLayer);
                                    } });
            }

            const RenderOutcome outcome = Render(kDefaultAssetsPath, settings, std::move(actions));
            AM_EXPECT(outcome.error.empty());

            // The scenario was reached: the new sound got the old sound's channel state.
            AM_EXPECT(probe->reused);

            // The stale events released the old layer and left the new sound alone.
            AM_EXPECT(probe->firstLayerState == eVoiceState::Idle);
            AM_EXPECT(probe->secondPlaying);
            AM_EXPECT_EQ(1ULL, probe->secondLayers);
            AM_EXPECT_EQ(1, probe->observedBegins);
            AM_EXPECT_EQ(0, probe->observedEnds);
            AM_EXPECT_EQ(0, probe->observedStops);
        }
    };

    AM_REGISTER_TEST(core_engine, stale_finished_event_does_not_touch_reused_channel);
} // namespace SparkyStudios::Audio::Amplitude::Tests
