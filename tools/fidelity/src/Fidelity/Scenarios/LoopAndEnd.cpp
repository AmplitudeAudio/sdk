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

#include <Fidelity/Analysis/Spectrum.h>
#include <Fidelity/AssetGenerator.h>
#include <Fidelity/Scenarios/Common.h>
#include <Fidelity/Scenarios/Playback.h>
#include <Fidelity/Scenarios/Timing.h>
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

                MeasureEndOfSound(capture, *spec, kLeadIn, outFrames, out);
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
