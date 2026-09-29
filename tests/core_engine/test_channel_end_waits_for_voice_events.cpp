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
        struct EndProbe
        {
            Channel channel;
            bool began = false;
            bool ended = false;
            bool stopped = false;
            bool stopSeenAfterEnd = false;
            int checks = 0;
            int stoppedEarly = 0;
        };

        void SetFlag(ChannelEventInfo info)
        {
            *static_cast<bool*>(info.m_userData) = true;
        }
    } // namespace

    // The audio thread may publish a voice's Finished state before the game thread dispatches its Ended and Finished
    // events. The lock-step render runs a check after every mix, before the frame that dispatches the events: the real
    // channel must keep playing until End fires, otherwise the engine could recycle the channel without End and Stop.
    AM_TEST_CASE(PureUnitTestCase, core_engine, channel_end_waits_for_voice_events)
    {
    public:
        void Run() override
        {
            constexpr std::uint64_t kPlayAt = 4800;
            constexpr std::uint64_t kFrameSamples = 800;

            const std::string sound = SoundName(*FindStimulus("sine_997_44100"), false);

            RenderSettings settings;
            settings.configFile = ConfigName("isolated", 1024, 48000, ".config.amconfig");
            settings.durationSamples = kPlayAt + 2 * 48000 + 24000;

            auto probe = std::make_shared<EndProbe>();
            std::vector<TimedAction> actions{ { kPlayAt, "play",
                                                [probe, sound]()
                                                {
                                                    probe->channel = amEngine->Play(amEngine->GetSoundHandle(sound));
                                                    probe->channel.On(eChannelEvent_Begin, &SetFlag, &probe->began);
                                                    probe->channel.On(eChannelEvent_End, &SetFlag, &probe->ended);
                                                    probe->channel.On(eChannelEvent_Stop, &SetFlag, &probe->stopped);
                                                } } };

            for (std::uint64_t at = kPlayAt + kFrameSamples; at < settings.durationSamples; at += kFrameSamples)
            {
                actions.push_back({ at, "check",
                                    [probe]()
                                    {
                                        if (probe->ended)
                                            probe->stopSeenAfterEnd = probe->stopped;

                                        if (!probe->began || probe->ended)
                                            return;

                                        ++probe->checks;
                                        if (!probe->channel.GetState()->GetRealChannel().Playing())
                                            ++probe->stoppedEarly;
                                    } });
            }

            const RenderOutcome outcome = Render(kDefaultAssetsPath, settings, std::move(actions));
            AM_EXPECT(outcome.error.empty());

            AM_EXPECT(probe->began);
            AM_EXPECT(probe->checks > 100);
            AM_EXPECT_EQ(0, probe->stoppedEarly);
            AM_EXPECT(probe->ended);
            AM_EXPECT(probe->stopSeenAfterEnd); // during playback, not from the engine teardown
        }
    };

    AM_REGISTER_TEST(core_engine, channel_end_waits_for_voice_events);
} // namespace SparkyStudios::Audio::Amplitude::Tests
