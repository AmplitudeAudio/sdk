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

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Fidelity/AssetGenerator.h>
#include <Fidelity/Scenarios/Common.h>
#include <Fidelity/Scenarios/Playback.h>
#include <Fidelity/Scenarios/Timing.h>
#include <Fidelity/Targets.h>

namespace SparkyStudios::Audio::Amplitude::Fidelity
{
    namespace
    {
        class StartStopScenario final : public Scenario
        {
        public:
            [[nodiscard]] std::string Id() const override
            {
                return "P2";
            }

            [[nodiscard]] std::string Title() const override
            {
                return "Start and stop";
            }

            [[nodiscard]] std::uint32_t Dimensions() const override
            {
                return kGridBlockSize | kGridFps;
            }

            [[nodiscard]] std::vector<std::string> Variants() const override
            {
                return { "default_fade", "zero_fade" };
            }

            void Measure(const RunContext& context, const GridPoint& point, const std::string& variant, Measurement& out) const override
            {
                const StimulusSpec& spec = *FindStimulus("sine_997_44100");
                const double fs = point.outputRate;
                const std::uint64_t stopAt = kLeadIn + Seconds(1.0, fs);
                const std::uint64_t duration = stopAt + Seconds(0.5, fs);
                const AmTime fade = variant == "default_fade" ? kMinFadeDuration : 0.0;
                const std::string sound = SoundName(spec, false);

                const ActionFactory actions = [sound, stopAt, fade]()
                {
                    auto channel = std::make_shared<Channel>();
                    return std::vector<TimedAction>{
                        { kLeadIn, "play",
                          [sound, channel]()
                          {
                              *channel = amEngine->Play(amEngine->GetSoundHandle(sound));
                          } },
                        { stopAt, "stop",
                          [channel, fade]()
                          {
                              channel->Stop(fade);
                          } },
                    };
                };

                Capture capture;
                if (!RenderDeterministic(context, IsolatedSettings(point, duration), actions, "", out, capture))
                    return;

                if (!RequireSignal(out, capture, kLeadIn, stopAt))
                {
                    out.capture = std::move(capture);
                    return;
                }

                AddIntegrityMetrics(out, capture, OnsetFrame(capture, kLeadIn) + Seconds(0.05, fs), stopAt);
                AddClickMetrics(out, capture, 4000.0, kLeadIn, duration);

                MeasureStopTiming(capture, spec, kLeadIn, stopAt, fade, out);
                out.capture = std::move(capture);
            }
        };

        class TransportScenario final : public Scenario
        {
        public:
            [[nodiscard]] std::string Id() const override
            {
                return "P3";
            }

            [[nodiscard]] std::string Title() const override
            {
                return "Pause, resume and seek";
            }

            [[nodiscard]] std::uint32_t Dimensions() const override
            {
                return kGridBlockSize | kGridFps;
            }

            [[nodiscard]] std::vector<std::string> Variants() const override
            {
                return { "pause_resume", "seek_forward", "seek_backward" };
            }

            void Measure(const RunContext& context, const GridPoint& point, const std::string& variant, Measurement& out) const override
            {
                const StimulusSpec& spec = *FindStimulus("chirp_44100");
                const double fs = point.outputRate;
                const std::string sound = SoundName(spec, false);
                const bool pause = variant == "pause_resume";
                const bool forward = variant == "seek_forward";

                const std::uint64_t first = kLeadIn + Seconds(pause || forward ? 1.0 : 2.0, fs);
                const std::uint64_t second = kLeadIn + Seconds(1.5, fs);
                const std::uint64_t duration = kLeadIn + Seconds(pause ? 2.5 : forward ? 1.8 : 2.8, fs);
                const double seekSeconds = forward ? 3.0 : 0.5;

                const ActionFactory actions = [sound, pause, first, second, seekSeconds]()
                {
                    auto channel = std::make_shared<Channel>();
                    std::vector<TimedAction> timeline = {
                        { kLeadIn, "play",
                          [sound, channel]()
                          {
                              *channel = amEngine->Play(amEngine->GetSoundHandle(sound));
                          } },
                    };

                    if (pause)
                    {
                        timeline.push_back(
                            { first, "pause", [channel]()
                              {
                                  channel->Pause();
                              } });
                        timeline.push_back(
                            { second, "resume", [channel]()
                              {
                                  channel->Resume();
                              } });
                    }
                    else
                    {
                        timeline.push_back(
                            { first, "seek", [channel, seekSeconds]()
                              {
                                  AM_UNUSED(channel->SetPlaybackPosition(seekSeconds * 1000.0));
                              } });
                    }

                    return timeline;
                };

                Capture capture;
                if (!RenderDeterministic(context, IsolatedSettings(point, duration), actions, "", out, capture))
                    return;

                if (!RequireSignal(out, capture, kLeadIn, first))
                {
                    out.capture = std::move(capture);
                    return;
                }

                AddIntegrityMetrics(out, capture, 0, 0);
                AddClickMetrics(out, capture, 4000.0, kLeadIn, duration);

                if (pause)
                    MeasurePauseResume(capture, spec, first, second, out);
                else
                    MeasureSeek(capture, spec, first, seekSeconds, out);

                out.capture = std::move(capture);
            }
        };
    } // namespace

    std::unique_ptr<Scenario> MakeStartStopScenario()
    {
        return std::make_unique<StartStopScenario>();
    }

    std::unique_ptr<Scenario> MakeTransportScenario()
    {
        return std::make_unique<TransportScenario>();
    }
} // namespace SparkyStudios::Audio::Amplitude::Fidelity
