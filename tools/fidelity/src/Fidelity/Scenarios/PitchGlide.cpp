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
#include <cstdint>
#include <memory>
#include <string>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Fidelity/Analysis/Pitch.h>
#include <Fidelity/Analysis/Sideband.h>
#include <Fidelity/Analysis/Spectrum.h>
#include <Fidelity/AssetGenerator.h>
#include <Fidelity/Scenarios/Common.h>
#include <Fidelity/Scenarios/Playback.h>
#include <Fidelity/Targets.h>

namespace SparkyStudios::Audio::Amplitude::Fidelity
{
    namespace
    {
        constexpr double kSourceHz = 997.0; // loop_sine_48000: 1000 Hz is exactly 48 samples per period, so it lands on
                                            // bin centres and hides any tuning error it would be there to expose.
        constexpr double kLow = 0.5;
        constexpr double kHigh = 2.0;
        constexpr double kHold = 1.0;
        constexpr double kGlide = 2.0;
        constexpr double kTail = 0.5;
        constexpr double kUpdateSeconds = 0.01;

        /// Commanded pitch at @p t seconds after the play: hold, glide up, hold, glide down, hold.
        double CommandedPitch(double t)
        {
            const double ratio = kHigh / kLow;
            if (t < kHold)
                return kLow;
            if (t < kHold + kGlide)
                return kLow * std::pow(ratio, (t - kHold) / kGlide);
            if (t < 2.0 * kHold + kGlide)
                return kHigh;
            if (t < 2.0 * kHold + 2.0 * kGlide)
                return kHigh * std::pow(ratio, -(t - 2.0 * kHold - kGlide) / kGlide);
            return kLow;
        }

        double TotalSeconds()
        {
            return 2.0 * kHold + 2.0 * kGlide + kTail;
        }

        ActionFactory Glide(double fs)
        {
            return [fs]()
            {
                std::vector<TimedAction> actions;
                actions.push_back(
                    { 0, "pitch", []()
                      {
                          amEngine->GetRtpcHandle(kGlideRtpcName)->SetValue(kLow);
                      } });
                actions.push_back(
                    { kLeadIn, "play", []()
                      {
                          AM_UNUSED(amEngine->Play(amEngine->GetSoundHandle(kGlideSoundName)));
                      } });

                for (double t = kUpdateSeconds; t < TotalSeconds(); t += kUpdateSeconds)
                {
                    const double pitch = CommandedPitch(t);
                    actions.push_back(
                        { kLeadIn + Seconds(t, fs), "pitch", [pitch]()
                          {
                              amEngine->GetRtpcHandle(kGlideRtpcName)->SetValue(pitch);
                          } });
                }

                return actions;
            };
        }

        void MeasureHold(
            const Signal& x, double fs, std::uint64_t begin, std::uint64_t end, double pitch, const std::string& name, Measurement& out)
        {
            SpectrumOptions options;
            options.fundamentalHz = kSourceHz * pitch;
            options.bandHighHz = std::min(20000.0, 0.45 * fs);
            const SpectrumResult r = AnalyzeSpectrum(std::span<const double>(x.data() + begin, end - begin), fs, options);

            out.Add(name + ".worstSpurDbc", r.worstSpurDbc, "dBc", Better::Lower, Targets::kMaxSidebandDbc);
            out.Add(
                name + ".frequencyErrorCents", std::abs(r.frequencyErrorCents), "cents", Better::Lower,
                Targets::kMaxPitchRmsDeviationCents);
        }

        /// Sidebands against the carrier, over one stretch of the glide.
        ///
        /// Reported, not gated: an eased ramp reads -73.7 / -68.8 dBc (up / down) on the quick grid against -76.5 /
        /// -71.0 for a straight one, too close to the interpolator's own block-rate content for a threshold.
        /// test_resampler_ratio_ramp_joins_smoothly gates the ramp shape instead.
        void MeasureGlideSidebands(
            const Signal& x,
            const Signal& expected,
            double fs,
            double blockRateHz,
            std::uint64_t begin,
            std::uint64_t end,
            const std::string& name,
            Measurement& out)
        {
            GlideSidebandOptions options;
            options.begin = static_cast<std::size_t>(begin);
            options.end = static_cast<std::size_t>(end);
            options.sampleRate = fs;
            options.blockRateHz = blockRateHz;

            const GlideSidebandResult r = AnalyzeGlideSidebands(x, expected, options);
            out.Add(name + ".sidebandDbc", r.sidebandDbc, "dBc", Better::Lower);
            out.Add(name + ".sidebandCarrierDbfs", r.carrierDbfs, "dBFS", Better::Lower);
        }

        class PitchGlideScenario final : public Scenario
        {
        public:
            [[nodiscard]] std::string Id() const override
            {
                return "P11";
            }

            [[nodiscard]] std::string Title() const override
            {
                return "Pitch glide";
            }

            [[nodiscard]] std::uint32_t Dimensions() const override
            {
                return kGridOutputRate;
            }

            [[nodiscard]] std::vector<std::string> Variants() const override
            {
                return { "loop_sine_48000" };
            }

            void Measure(const RunContext& context, const GridPoint& point, const std::string& variant, Measurement& out) const override
            {
                const double fs = point.outputRate;
                const std::uint64_t playEnd = kLeadIn + Seconds(TotalSeconds(), fs);

                Capture capture;
                if (!RenderDeterministic(context, IsolatedSettings(point, playEnd + Seconds(0.1, fs)), Glide(fs), "", out, capture))
                    return;

                if (!RequireSignal(out, capture, kLeadIn, playEnd))
                {
                    out.capture = std::move(capture);
                    return;
                }

                // Two timelines are in play and they are not offset equally across output rates: the commands go out on
                // the action timeline (kLeadIn + Seconds(t, fs)), while the audio starts at the observed onset --
                // 321 frames after kLeadIn at 48 kHz against 1345 at 44.1 kHz, a whole 1024-frame block apart.
                // Anything that compares the render against a *command* is anchored on the action timeline, so the
                // mismatch reported is the engine's own lag and not the anchor. Anything that reads the render itself --
                // the click window and the hold windows -- stays on the observed onset.
                const std::uint64_t onset = OnsetFrame(capture, kLeadIn);
                AddClickMetrics(out, capture, 4000.0, onset + Seconds(0.05, fs), playEnd);

                const Signal x = ChannelSignal(capture, 0);
                const auto at = [&](double t)
                {
                    return onset + Seconds(t, fs);
                };
                const auto commanded = [&](double t)
                {
                    return kLeadIn + Seconds(t, fs);
                };

                // The holds start a quarter second in, once the per-block pitch easing has settled.
                MeasureHold(x, fs, at(0.25), at(kHold - 0.05), kLow, "holdLow", out);
                MeasureHold(x, fs, at(kHold + kGlide + 0.25), at(2.0 * kHold + kGlide - 0.05), kHigh, "holdHigh", out);

                Signal expected(x.size(), 0.0);
                for (std::size_t i = 0; i < expected.size(); ++i)
                {
                    const double t = (static_cast<double>(i) - static_cast<double>(kLeadIn)) / fs;
                    expected[i] = kSourceHz * CommandedPitch(std::max(0.0, t));
                }

                PitchOptions options;
                options.begin = commanded(kHold);
                options.end = commanded(2.0 * kHold + 2.0 * kGlide);
                options.expectedHz = std::move(expected);
                const PitchResult pitch = AnalyzePitch(x, fs, options);
                out.Add("glide.rmsDeviationCents", pitch.rmsDeviationCents, "cents", Better::Lower);
                out.Add("glide.largestStepCents", pitch.largestStepCents, "cents", Better::Lower);

                // Only the glides ramp the ratio. Each is measured on its own, 50 ms in from either end: a hold inside
                // the window would add carrier and dilute the sidebands.
                const double blockRateHz = fs / static_cast<double>(point.blockSize);
                constexpr double kGlideTrim = 0.05;
                MeasureGlideSidebands(
                    x, pitch.frequencyHz, fs, blockRateHz, at(kHold + kGlideTrim), at(kHold + kGlide - kGlideTrim), "glideUp", out);
                MeasureGlideSidebands(
                    x, pitch.frequencyHz, fs, blockRateHz, at(2.0 * kHold + kGlide + kGlideTrim),
                    at(2.0 * kHold + 2.0 * kGlide - kGlideTrim), "glideDown", out);

                out.capture = std::move(capture);
            }
        };
    } // namespace

    std::unique_ptr<Scenario> MakePitchGlideScenario()
    {
        return std::make_unique<PitchGlideScenario>();
    }
} // namespace SparkyStudios::Audio::Amplitude::Fidelity
