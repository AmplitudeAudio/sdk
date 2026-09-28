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

#include <algorithm>
#include <cmath>
#include <memory>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Fidelity/Analysis/Analytic.h>
#include <Fidelity/Analysis/Envelope.h>
#include <Fidelity/Analysis/Pitch.h>
#include <Fidelity/AssetGenerator.h>
#include <Fidelity/Scenarios/Common.h>
#include <Fidelity/Scenarios/Playback.h>
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

                AddIntegrityMetrics(out, capture, kLeadIn + Seconds(0.05, fs), stopAt);
                AddClickMetrics(out, capture, 4000.0, kLeadIn, duration);

                const Signal x = ChannelSignal(capture, 0);
                const Signal envelope = Envelope(x);
                const double level = spec.amplitude * CenterPanGain();

                const double onset = CrossingTime(envelope, 0.5 * level, static_cast<double>(kLeadIn), true);
                if (onset >= 0.0)
                    out.Add("start.latencyMs", (onset - static_cast<double>(kLeadIn)) / fs * 1000.0, "ms", Better::Lower);

                const double departure = CrossingTime(envelope, 0.999 * level, static_cast<double>(stopAt), false);
                if (departure < 0.0)
                {
                    out.error = "the stop fade was not found";
                    out.capture = std::move(capture);
                    return;
                }

                out.Add("stop.latencyMs", (departure - static_cast<double>(stopAt)) / fs * 1000.0, "ms", Better::Lower);

                const FadeTiming timing = MeasureFade(envelope, fs, level, 0.0, departure);
                if (timing.found)
                    out.Add("stop.fadeMs", timing.durationMs, "ms", Better::Lower);

                if (fade > 0.0)
                {
                    const double fadeSamples = fade / 1000.0 * fs;
                    double worst = 0.0;
                    for (auto i = static_cast<std::size_t>(std::ceil(departure)); i < envelope.size(); ++i)
                    {
                        const double expected = level * (1.0 - (static_cast<double>(i) - departure) / fadeSamples);
                        if (expected < 0.01 * level)
                            break;

                        worst = std::max(worst, std::abs(DbFromAmplitude(envelope[i] / expected)));
                    }

                    out.Add("stop.fadeCurveErrorDb", worst, "dB", Better::Lower, Targets::kMaxRampErrorDb);
                }

                const std::uint64_t tailBegin = stopAt + Seconds(0.1, fs);
                if (tailBegin < x.size())
                    out.Add(
                        "stop.tailDbfs", DbFromAmplitude(Peak(std::span<const double>(x.data() + tailBegin, x.size() - tailBegin))), "dBFS",
                        Better::Lower, Targets::kMaxTailDbfs);

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

                const Signal x = ChannelSignal(capture, 0);
                const Signal envelope = Envelope(x);
                const double level = spec.amplitude * CenterPanGain();
                const Signal tau = EstimateChirpSourceTime(x, fs, ChirpModelOf(spec), 0.9 * level);

                // Source position minus output time, in output samples; constant while the sound plays at speed 1.
                const auto offset = [&](std::uint64_t from, std::uint64_t to)
                {
                    std::vector<double> values;
                    for (std::uint64_t i = from; i < std::min<std::uint64_t>(to, tau.size()); ++i)
                        values.push_back(tau[i] * fs - static_cast<double>(i));

                    return MedianFinite(std::move(values));
                };

                if (pause)
                {
                    const double before = offset(first - Seconds(0.3, fs), first - Seconds(0.05, fs));
                    const double after = offset(second + Seconds(0.1, fs), second + Seconds(0.4, fs));
                    const double stopEdge = FadeZero(envelope, level, static_cast<double>(first), false);
                    const double startEdge = FadeZero(envelope, level, static_cast<double>(second), true);

                    if (!std::isfinite(before) || !std::isfinite(after) || stopEdge < 0.0 || startEdge < 0.0)
                    {
                        out.error = "cannot locate the pause and resume edges";
                        out.capture = std::move(capture);
                        return;
                    }

                    out.Add(
                        "resume.positionErrorSamples", std::abs((after + startEdge) - (before + stopEdge)), "samples", Better::Lower,
                        Targets::kMaxResumePositionErrorSamples);

                    const FadeTiming pauseFade = MeasureFade(envelope, fs, level, 0.0, static_cast<double>(first));
                    const FadeTiming resumeFade = MeasureFade(envelope, fs, 0.0, level, static_cast<double>(second));
                    if (pauseFade.found)
                        out.Add("pause.fadeMs", pauseFade.durationMs, "ms", Better::Lower);
                    if (resumeFade.found)
                        out.Add("resume.fadeMs", resumeFade.durationMs, "ms", Better::Lower);
                }
                else
                {
                    const double after = offset(first + Seconds(0.2, fs), first + Seconds(0.6, fs));
                    if (!std::isfinite(after))
                    {
                        out.error = "cannot track the chirp after the seek";
                        out.capture = std::move(capture);
                        return;
                    }

                    // The output sample where the source was at the seek target.
                    const double effect = seekSeconds * fs - after;
                    out.Add("seek.latencyMs", (effect - static_cast<double>(first)) / fs * 1000.0, "ms", Better::Lower);
                }

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
