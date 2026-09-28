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

#include <Fidelity/Analysis/Analytic.h>
#include <Fidelity/Analysis/Envelope.h>
#include <Fidelity/Analysis/Spectrum.h>
#include <Fidelity/AssetGenerator.h>
#include <Fidelity/Scenarios/Common.h>
#include <Fidelity/Scenarios/Playback.h>
#include <Fidelity/Targets.h>

namespace SparkyStudios::Audio::Amplitude::Fidelity
{
    namespace
    {
        class LoopSeamScenario final : public Scenario
        {
        public:
            [[nodiscard]] std::string Id() const override
            {
                return "P4";
            }

            [[nodiscard]] std::string Title() const override
            {
                return "Loop seam";
            }

            [[nodiscard]] std::uint32_t Dimensions() const override
            {
                return kGridBlockSize;
            }

            [[nodiscard]] std::vector<std::string> Variants() const override
            {
                return StimulusNames({ StimulusKind::LoopSine });
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
                const std::uint64_t duration = kLeadIn + Seconds(4.5, fs);

                Capture capture;
                if (!RenderDeterministic(context, IsolatedSettings(point, duration), PlayOnly(SoundName(*spec, false)), "", out, capture))
                    return;

                if (!RequireSignal(out, capture, kLeadIn, duration))
                {
                    out.capture = std::move(capture);
                    return;
                }

                const std::uint64_t begin = kLeadIn + Seconds(0.05, fs);
                AddIntegrityMetrics(out, capture, begin, duration - 2048);
                AddClickMetrics(out, capture, 4000.0, begin, duration);

                const Signal x = ChannelSignal(capture, 0);
                const std::uint64_t steadyBegin = kLeadIn + Seconds(0.2, fs);
                const std::uint64_t steadyEnd = duration - Seconds(0.2, fs);
                SpectrumOptions options;
                options.fundamentalHz = spec->frequencyHz;
                const SpectrumResult r =
                    AnalyzeSpectrum(std::span<const double>(x.data() + steadyBegin, steadyEnd - steadyBegin), fs, options);
                out.Add("spectrum.thdnDb", r.thdnDb, "dB", Better::Lower, Targets::kMaxThdnDb);

                out.capture = std::move(capture);
            }
        };

        class EndOfSoundScenario final : public Scenario
        {
        public:
            [[nodiscard]] std::string Id() const override
            {
                return "P7";
            }

            [[nodiscard]] std::string Title() const override
            {
                return "End of sound";
            }

            [[nodiscard]] std::uint32_t Dimensions() const override
            {
                return kGridBlockSize;
            }

            [[nodiscard]] std::vector<std::string> Variants() const override
            {
                return { "sine_997_22050", "sine_997_44100", "sine_997_48000", "sine_997_96000" };
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
                const std::uint64_t outFrames = OutputFrames(*spec, point.outputRate);
                const std::uint64_t duration = kLeadIn + outFrames + Seconds(0.5, fs);

                Capture capture;
                if (!RenderDeterministic(context, IsolatedSettings(point, duration), PlayOnly(SoundName(*spec, false)), "", out, capture))
                    return;

                if (!RequireSignal(out, capture, kLeadIn, kLeadIn + outFrames))
                {
                    out.capture = std::move(capture);
                    return;
                }

                AddClickMetrics(out, capture, 4000.0, kLeadIn, duration);

                const Signal x = ChannelSignal(capture, 0);
                const Signal envelope = Envelope(x);
                const double level = spec->amplitude * CenterPanGain();
                const double ratio = fs / spec->sampleRate;

                const double start = CrossingTime(envelope, 0.5 * level, static_cast<double>(kLeadIn), true);
                const double end =
                    start < 0.0 ? -1.0 : CrossingTime(envelope, 0.5 * level, start + static_cast<double>(outFrames) / 2.0, false);
                if (start < 0.0 || end < 0.0)
                {
                    out.error = "cannot locate the start and end of the sound";
                    out.capture = std::move(capture);
                    return;
                }

                const double fadeOut = static_cast<double>(FadeFrames(*spec)) * ratio;
                AddIntegrityMetrics(
                    out, capture, static_cast<std::uint64_t>(start + fadeOut), static_cast<std::uint64_t>(std::max(start, end - fadeOut)));

                const double expected = static_cast<double>(StimulusFrameCount(*spec) - 1 - FadeFrames(*spec)) * ratio;
                out.Add(
                    "length.errorSamples", std::abs((end - start) - expected), "samples", Better::Lower, Targets::kMaxLengthErrorSamples);

                const auto tailBegin = static_cast<std::size_t>(end + fadeOut + 256.0);
                if (tailBegin < x.size())
                    out.Add(
                        "tail.peakDbfs", DbFromAmplitude(Peak(std::span<const double>(x.data() + tailBegin, x.size() - tailBegin))), "dBFS",
                        Better::Lower, Targets::kMaxTailDbfs);

                out.capture = std::move(capture);
            }
        };
    } // namespace

    std::unique_ptr<Scenario> MakeLoopSeamScenario()
    {
        return std::make_unique<LoopSeamScenario>();
    }

    std::unique_ptr<Scenario> MakeEndOfSoundScenario()
    {
        return std::make_unique<EndOfSoundScenario>();
    }
} // namespace SparkyStudios::Audio::Amplitude::Fidelity
