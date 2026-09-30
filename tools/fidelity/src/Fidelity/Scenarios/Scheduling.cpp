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

#include <cmath>
#include <memory>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Mixer/Voice/VoiceTypes.h>

#include <Fidelity/Analysis/Envelope.h>
#include <Fidelity/AssetGenerator.h>
#include <Fidelity/Scenarios/Common.h>
#include <Fidelity/Scenarios/Playback.h>
#include <Fidelity/Targets.h>

namespace SparkyStudios::Audio::Amplitude::Fidelity
{
    namespace
    {
        /// Lands mid-block at every grid block size.
        constexpr std::uint64_t kScheduleOffset = 12000 + 137;

        /// B in the splice: below the click detector's 4 kHz high-pass, so only splice artefacts reach it.
        constexpr const char* kSpliceIncoming = "sine_100_48000";

        class ScheduledStartScenario final : public Scenario
        {
        public:
            [[nodiscard]] std::string Id() const override
            {
                return "P10";
            }

            [[nodiscard]] std::string Title() const override
            {
                return "Scheduled start";
            }

            [[nodiscard]] std::uint32_t Dimensions() const override
            {
                return kGridBlockSize | kGridFps;
            }

            [[nodiscard]] std::vector<std::string> Variants() const override
            {
                return { "sine_997_48000", "sine_997_44100", "sine_997_96000" };
            }

            void Measure(const RunContext& context, const GridPoint& point, const std::string& variant, Measurement& out) const override
            {
                const StimulusSpec* spec = FindStimulus(variant);
                if (spec == nullptr)
                {
                    out.error = "unknown stimulus " + variant;
                    return;
                }

                const double fs = point.outputRate;
                const std::uint64_t duration = kLeadIn + kScheduleOffset + Seconds(1.0, fs);
                const std::string sound = SoundName(*spec, false);
                auto target = std::make_shared<std::uint64_t>(0);

                const ActionFactory actions = [sound, target]()
                {
                    auto channel = std::make_shared<Channel>();
                    return std::vector<TimedAction>{
                        { kLeadIn, "play",
                          [sound, channel, target]()
                          {
                              *channel = amEngine->Play(amEngine->GetSoundHandle(sound));
                              // The capture starts with the engine, so capture frames are audio-clock frames.
                              *target = amEngine->GetAudioClock() + kScheduleOffset;
                              AM_UNUSED(channel->ScheduleStart(*target));
                          } },
                    };
                };

                Capture capture;
                if (!RenderDeterministic(context, IsolatedSettings(point, duration), actions, "", out, capture))
                    return;

                if (!RequireSignal(out, capture, *target, duration))
                {
                    out.capture = std::move(capture);
                    return;
                }

                const Signal x = ChannelSignal(capture, 0);
                const Signal envelope = TimingEnvelope(x);
                const double plateau = PlateauLevel(envelope, *target + Seconds(0.3, fs), *target + Seconds(0.6, fs));
                const double onset = SustainedCrossing(envelope, 0.5 * plateau, static_cast<double>(kLeadIn), true, kTimingHold);

                if (plateau <= 0.0 || onset < 0.0)
                {
                    out.error = "cannot locate the scheduled onset";
                    out.capture = std::move(capture);
                    return;
                }

                // The asset fades in on its own: its 50 % point is half its fade after the first sample.
                const double assetHalfFade = 0.5 * static_cast<double>(FadeFrames(*spec)) * fs / spec->sampleRate;
                out.Add(
                    "schedule.startErrorSamples", std::abs(onset - assetHalfFade - static_cast<double>(*target)), "samples", Better::Lower,
                    Targets::kMaxScheduleErrorSamples);

                AddIntegrityMetrics(out, capture, *target + Seconds(0.05, fs), duration - 2048);
                AddClickMetrics(out, capture, 4000.0, kLeadIn, duration);
                out.capture = std::move(capture);
            }
        };

        class ScheduledSpliceScenario final : public Scenario
        {
        public:
            [[nodiscard]] std::string Id() const override
            {
                return "P9";
            }

            [[nodiscard]] std::string Title() const override
            {
                return "Scheduled splice";
            }

            [[nodiscard]] std::uint32_t Dimensions() const override
            {
                return kGridBlockSize | kGridFps;
            }

            [[nodiscard]] std::vector<std::string> Variants() const override
            {
                return { "sine_997_48000", "sine_997_44100" };
            }

            void Measure(const RunContext& context, const GridPoint& point, const std::string& variant, Measurement& out) const override
            {
                const StimulusSpec* spec = FindStimulus(variant);
                if (spec == nullptr)
                {
                    out.error = "unknown stimulus " + variant;
                    return;
                }

                const double fs = point.outputRate;
                const std::uint64_t spliceAction = kLeadIn + Seconds(0.5, fs);
                const std::uint64_t duration = spliceAction + kScheduleOffset + Seconds(0.5, fs);
                const std::string outgoing = SoundName(*spec, false);
                const std::string incoming = SoundName(*FindStimulus(kSpliceIncoming), false);
                auto frame = std::make_shared<std::uint64_t>(0);

                const auto timeline = [outgoing, incoming, spliceAction, frame](bool withIncoming)
                {
                    return [outgoing, incoming, spliceAction, frame, withIncoming]()
                    {
                        auto a = std::make_shared<Channel>();
                        auto b = std::make_shared<Channel>();
                        return std::vector<TimedAction>{
                            { kLeadIn, "play",
                              [outgoing, a]()
                              {
                                  *a = amEngine->Play(amEngine->GetSoundHandle(outgoing));
                              } },
                            { spliceAction, "splice",
                              [incoming, a, b, frame, withIncoming]()
                              {
                                  *frame = amEngine->GetAudioClock() + kScheduleOffset;
                                  a->Stop(0.0, *frame);
                                  if (!withIncoming)
                                      return;

                                  *b = amEngine->Play(amEngine->GetSoundHandle(incoming));
                                  AM_UNUSED(b->ScheduleStart(*frame));
                              } },
                        };
                    };
                };

                // 1. The outgoing sound alone: its scheduled stop starts exactly at the splice frame.
                Capture stopOnly;
                if (!RenderDeterministic(context, IsolatedSettings(point, duration), timeline(false), "stop", out, stopOnly))
                    return;

                const Signal x = ChannelSignal(stopOnly, 0);
                const Signal envelope = TimingEnvelope(x);
                const double plateau = PlateauLevel(envelope, *frame - Seconds(0.3, fs), *frame - Seconds(0.02, fs));
                const double half = SustainedCrossing(envelope, 0.5 * plateau, static_cast<double>(*frame - Seconds(0.01, fs)), false, kTimingHold);
                if (plateau <= 0.0 || half < 0.0)
                {
                    out.error = "cannot locate the scheduled stop";
                    return;
                }

                // The de-click is a raised cosine, symmetric about its 50 % point.
                const double declick = kDeclickFade / 1000.0 * fs;
                out.Add(
                    "schedule.stopErrorSamples", std::abs(half - 0.5 * declick - static_cast<double>(*frame)), "samples", Better::Lower,
                    Targets::kMaxScheduleErrorSamples);

                // 2. The splice: the incoming sound starts on the same frame. No click anywhere around it.
                Capture splice;
                if (!RenderDeterministic(context, IsolatedSettings(point, duration), timeline(true), "splice", out, splice))
                    return;

                AddClickMetrics(out, splice, 4000.0, *frame - Seconds(0.1, fs), *frame + Seconds(0.1, fs));
                AddIntegrityMetrics(out, splice, OnsetFrame(splice, kLeadIn) + Seconds(0.05, fs), duration - 2048);
                out.capture = std::move(splice);
            }
        };
    } // namespace

    std::unique_ptr<Scenario> MakeScheduledSpliceScenario()
    {
        return std::make_unique<ScheduledSpliceScenario>();
    }

    std::unique_ptr<Scenario> MakeScheduledStartScenario()
    {
        return std::make_unique<ScheduledStartScenario>();
    }
} // namespace SparkyStudios::Audio::Amplitude::Fidelity
