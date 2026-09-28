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

#include <Fidelity/Analysis/FrequencyResponse.h>
#include <Fidelity/Analysis/Spectrum.h>
#include <Fidelity/AssetGenerator.h>
#include <Fidelity/Scenarios/Common.h>
#include <Fidelity/Scenarios/Playback.h>
#include <Fidelity/Targets.h>

namespace SparkyStudios::Audio::Amplitude::Fidelity
{
    namespace
    {
        void MeasureTone(const StimulusSpec& spec, std::span<const double> steady, double fs, const std::string& suffix, Measurement& out)
        {
            const double f = spec.frequencyHz;
            if (f < fs / 2.0)
            {
                SpectrumOptions options;
                options.fundamentalHz = f;
                options.bandHighHz = std::min(20000.0, 0.45 * fs);
                const SpectrumResult r = AnalyzeSpectrum(steady, fs, options);

                out.Add(
                    "spectrum.levelErrorDb" + suffix, r.fundamentalDbfs - DbFromAmplitude(spec.amplitude * CenterPanGain()), "dB",
                    Better::Lower);
                out.Add("spectrum.thdDb" + suffix, r.thdDb, "dB", Better::Lower);
                out.Add("spectrum.thdnDb" + suffix, r.thdnDb, "dB", Better::Lower, Targets::kMaxThdnDb);
                out.Add("spectrum.worstSpurDbc" + suffix, r.worstSpurDbc, "dBc", Better::Lower, Targets::kMaxSpurDbc);
                out.Add("spectrum.worstSpurHz" + suffix, r.worstSpurHz, "Hz", Better::Lower);
                out.Add("spectrum.noiseFloorDbfs" + suffix, r.noiseFloorDbfs, "dBFS", Better::Lower);
                out.Add(
                    "spectrum.frequencyErrorCents" + suffix, std::abs(r.frequencyErrorCents), "cents", Better::Lower,
                    Targets::kMaxPitchRmsDeviationCents);
                return;
            }

            // The tone lies above the output band: anything left of it was folded back by the resampler.
            out.Add("alias.levelDbc" + suffix, AliasLevelDbc(steady, fs, spec), "dBc", Better::Lower, Targets::kMaxSpurDbc);
            out.Add("alias.totalDbfs" + suffix, DbFromAmplitude(Rms(steady)), "dBFS", Better::Lower);
        }

        class ResamplingQualityScenario final : public Scenario
        {
        public:
            [[nodiscard]] std::string Id() const override
            {
                return "P1";
            }

            [[nodiscard]] std::string Title() const override
            {
                return "Resampling quality";
            }

            [[nodiscard]] std::uint32_t Dimensions() const override
            {
                return kGridOutputRate;
            }

            [[nodiscard]] std::vector<std::string> Variants() const override
            {
                return StimulusNames({ StimulusKind::Sine, StimulusKind::LogSweep, StimulusKind::PinkNoise });
            }

            void Measure(const RunContext& context, const GridPoint& point, const std::string& variant, Measurement& out) const override
            {
                const StimulusSpec* spec = FindStimulus(variant);
                if (spec == nullptr)
                {
                    out.error = "unknown stimulus " + variant;
                    return;
                }

                const std::uint64_t outFrames = OutputFrames(*spec, point.outputRate);
                const std::uint64_t duration = kLeadIn + outFrames + Seconds(0.1, point.outputRate);

                Capture capture;
                auto played = std::make_shared<bool>(false);
                if (!RenderDeterministic(
                        context, IsolatedSettings(point, duration), PlayOnly(SoundName(*spec, false), played), "", out, capture))
                    return;

                const double fs = capture.sampleRate;
                const std::uint64_t playEnd = kLeadIn + outFrames;
                const bool expectsSilence = spec->kind == StimulusKind::Sine && spec->frequencyHz >= fs / 2.0;
                if (expectsSilence && !*played)
                {
                    // Silence is the correct output here, so it cannot prove the sound played: the channel must.
                    out.error = "no signal: the sound did not play";
                    out.capture = std::move(capture);
                    return;
                }

                if (!expectsSilence && !RequireSignal(out, capture, kLeadIn, playEnd))
                {
                    out.capture = std::move(capture);
                    return;
                }

                AddIntegrityMetrics(
                    out, capture, expectsSilence ? 0 : kLeadIn + Seconds(0.05, fs), expectsSilence ? 0 : playEnd - Seconds(0.05, fs));

                const std::uint64_t margin = Seconds(spec->fadeSeconds + 0.1, fs);
                const std::uint64_t begin = kLeadIn + margin;
                const std::uint64_t end = playEnd - margin;
                const std::size_t analysed = spec->channels == 2 ? 2 : 1;

                for (std::size_t c = 0; c < analysed; ++c)
                {
                    const std::string suffix = analysed == 2 ? (c == 0 ? ".L" : ".R") : "";
                    const Signal x = ChannelSignal(capture, c);
                    const std::span<const double> steady(x.data() + begin, end - begin);

                    switch (spec->kind)
                    {
                    case StimulusKind::Sine:
                        MeasureTone(*spec, steady, fs, suffix, out);
                        break;
                    case StimulusKind::LogSweep:
                        {
                            ResponseOptions options;
                            options.passbandHighHz = 0.9 * std::min(spec->sampleRate / 2.0, fs / 2.0);
                            const ResponseResult r = AnalyzeFrequencyResponse(x, fs, SweepModelOf(*spec), options);
                            out.Add("response.rippleDb" + suffix, r.rippleDb, "dB", Better::Lower, Targets::kMaxPassbandRippleDb);
                            out.Add("response.minus3dBHz" + suffix, r.minus3dBHz, "Hz", Better::Higher);
                            break;
                        }
                    case StimulusKind::PinkNoise:
                        out.Add(
                            "level.errorDb" + suffix, DbFromAmplitude(Rms(steady)) - DbFromAmplitude(spec->amplitude * CenterPanGain()),
                            "dB", Better::Lower);
                        break;
                    default:
                        break;
                    }
                }

                if (spec->kind == StimulusKind::Sine && 2.0 * spec->frequencyHz <= 0.45 * fs)
                    AddClickMetrics(out, capture, std::max(4000.0, 2.0 * spec->frequencyHz), begin, end);

                out.capture = std::move(capture);
            }
        };
    } // namespace

    std::unique_ptr<Scenario> MakeResamplingQualityScenario()
    {
        return std::make_unique<ResamplingQualityScenario>();
    }
} // namespace SparkyStudios::Audio::Amplitude::Fidelity
