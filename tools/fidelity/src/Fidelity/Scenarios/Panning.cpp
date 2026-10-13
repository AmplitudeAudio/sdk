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
#include <limits>
#include <memory>
#include <span>
#include <string>
#include <vector>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Fidelity/Analysis/Integrity.h>
#include <Fidelity/AssetGenerator.h>
#include <Fidelity/Scenarios/Common.h>
#include <Fidelity/Scenarios/Playback.h>
#include <Fidelity/Targets.h>

namespace SparkyStudios::Audio::Amplitude::Fidelity
{
    namespace
    {
        constexpr double kStepSeconds = 1.0 / 60.0;

        struct OutputLayout
        {
            std::string label;
            std::uint16_t channels;
            std::vector<double> azimuths; ///< Per output channel, degrees, positive left; NaN for the LFE.
        };

        const std::vector<OutputLayout>& Layouts()
        {
            static const std::vector<OutputLayout> layouts{
                { "stereo", 2, { 30.0, -30.0 } },
                { "surround51", 6, { 30.0, -30.0, 0.0, std::numeric_limits<double>::quiet_NaN(), 110.0, -110.0 } },
            };
            return layouts;
        }

        const OutputLayout& FindLayout(const std::string& label)
        {
            for (const OutputLayout& layout : Layouts())
                if (label.rfind(layout.label, 0) == 0)
                    return layout;

            return Layouts()[0];
        }

        RenderSettings SettingsFor(const OutputLayout& layout, const GridPoint& point, std::uint64_t duration)
        {
            RenderSettings settings = IsolatedSettings(point, duration);
            ConfigVariant variant{ point.blockSize, point.outputRate, layout.channels };
            settings.configFile = ConfigName(ConfigKind(variant), point.blockSize, point.outputRate, ".config.amconfig");
            return settings;
        }

        bool RequireAnySignal(Measurement& out, const Capture& capture, std::uint64_t begin, std::uint64_t end)
        {
            const std::vector<double> powers = ChannelPowers(capture, begin, end);
            if (*std::max_element(powers.begin(), powers.end()) > 1e-8)
                return true;

            out.error = "no signal on any channel";
            out.capture = capture;
            return false;
        }

        class PanLawScenario final : public Scenario
        {
        public:
            [[nodiscard]] std::string Id() const override
            {
                return "P12";
            }

            [[nodiscard]] std::string Title() const override
            {
                return "Pan law";
            }

            [[nodiscard]] std::uint32_t Dimensions() const override
            {
                return 0;
            }

            [[nodiscard]] std::vector<std::string> Variants() const override
            {
                std::vector<std::string> variants;
                for (const OutputLayout& layout : Layouts())
                    for (const int angle : { 0, 30, -30, 90, -90, 180 })
                        variants.push_back(layout.label + "_az" + std::to_string(angle));

                variants.push_back("f8_stereo_asset");
                return variants;
            }

            void Measure(const RunContext& context, const GridPoint& point, const std::string& variant, Measurement& out) const override
            {
                if (variant == "f8_stereo_asset")
                {
                    MeasureF8(context, point, out);
                    return;
                }

                const double fs = point.outputRate;
                const OutputLayout& layout = FindLayout(variant);
                const double angle = std::stod(variant.substr(variant.find("_az") + 3));
                const double amplitude = FindStimulus("loop_sine_48000")->amplitude;
                const std::uint64_t playEnd = kLeadIn + Seconds(1.5, fs);

                const ActionFactory factory = [angle]()
                {
                    return std::vector<TimedAction>{ SceneSetup(),
                                                     { kLeadIn, "play", [angle]()
                                                       {
                                                           AM_UNUSED(amEngine->Play(amEngine->GetSoundHandle(kPositionalSineName), LocationAt(angle)));
                                                       } } };
                };

                Capture capture;
                if (!RenderDeterministic(context, SettingsFor(layout, point, playEnd), factory, "", out, capture))
                    return;

                const std::uint64_t begin = OnsetFrame(capture, kLeadIn) + Seconds(0.25, fs);
                const std::uint64_t end = playEnd - Seconds(0.05, fs);
                if (!RequireAnySignal(out, capture, begin, end))
                    return;

                const std::vector<double> expected = AnalyticPanGains(angle, layout.azimuths);
                const std::vector<double> powers = ChannelPowers(capture, begin, end);

                double worstError = 0.0;
                double worstCrosstalk = -200.0;
                for (std::size_t c = 0; c < powers.size(); ++c)
                {
                    // A sine's peak is its RMS times sqrt(2).
                    const double levelDb = DbFromAmplitude(std::sqrt(2.0 * powers[c]));
                    if (expected[c] > 1e-3)
                        worstError = std::max(worstError, std::abs(levelDb - DbFromAmplitude(amplitude * expected[c])));
                    else
                        worstCrosstalk = std::max(worstCrosstalk, std::max(levelDb - DbFromAmplitude(amplitude), -200.0));
                }

                out.Add("pan.levelErrorDb", worstError, "dB", Better::Lower, Targets::kMaxPanLevelErrorDb);
                out.Add("pan.crosstalkDbc", worstCrosstalk, "dBc", Better::Lower, Targets::kMaxPanCrosstalkDbc);
                AddPanIntegrityMetrics(out, capture, begin, end);
                out.capture = std::move(capture);
            }

        private:
            /// F8: a stereo asset (the same tone on both channels) and the mono asset of that tone, both 2D. The stereo
            /// asset's mono fold-down is the mono asset raised by 3.01 dB, so it plays 3.01 dB louder per side.
            static void MeasureF8(const RunContext& context, const GridPoint& point, Measurement& out)
            {
                const double fs = point.outputRate;
                const StimulusSpec* mono = FindStimulus("sine_997_48000");
                const StimulusSpec* stereo = FindStimulus("sine_997_48000_stereo");
                const std::uint64_t playEnd = kLeadIn + OutputFrames(*mono, point.outputRate);

                Capture monoCapture;
                Capture stereoCapture;
                if (!RenderDeterministic(context, IsolatedSettings(point, playEnd), PlayOnly(SoundName(*mono, false)), "mono", out, monoCapture) ||
                    !RenderDeterministic(
                        context, IsolatedSettings(point, playEnd), PlayOnly(SoundName(*stereo, false)), "stereo", out, stereoCapture))
                    return;

                const std::uint64_t begin = OnsetFrame(monoCapture, kLeadIn) + Seconds(0.25, fs);
                const std::uint64_t end = playEnd - Seconds(0.25, fs);
                if (!RequireAnySignal(out, monoCapture, begin, end) || !RequireAnySignal(out, stereoCapture, begin, end))
                    return;

                const double monoDb = DbFromPower(ChannelPowers(monoCapture, begin, end)[0]);
                const double stereoDb = DbFromPower(ChannelPowers(stereoCapture, begin, end)[0]);
                out.Add("f8.levelErrorDb", std::abs(stereoDb - monoDb - 20.0 * std::log10(std::sqrt(2.0))), "dB", Better::Lower,
                        Targets::kMaxPanLevelErrorDb);
                out.capture = std::move(stereoCapture);
            }
        };

        class OrbitScenario final : public Scenario
        {
        public:
            static constexpr double kOrbitSeconds = 4.0;

            [[nodiscard]] std::string Id() const override
            {
                return "S1";
            }

            [[nodiscard]] std::string Title() const override
            {
                return "Orbit";
            }

            /// The surround configs exist at block 1024, 48 kHz only: the quick grid point.
            [[nodiscard]] std::uint32_t Dimensions() const override
            {
                return 0;
            }

            [[nodiscard]] std::vector<std::string> Variants() const override
            {
                return { "stereo", "surround51" };
            }

            void Measure(const RunContext& context, const GridPoint& point, const std::string& variant, Measurement& out) const override
            {
                const double fs = point.outputRate;
                const OutputLayout& layout = FindLayout(variant);
                const std::uint64_t playEnd = kLeadIn + Seconds(kOrbitSeconds, fs);

                const ActionFactory factory = [fs]()
                {
                    auto channel = std::make_shared<Channel>();
                    std::vector<TimedAction> actions{ SceneSetup(),
                                                      { kLeadIn, "play", [channel]()
                                                        {
                                                            *channel = amEngine->Play(amEngine->GetSoundHandle(kPositionalSineName), LocationAt(0.0));
                                                        } } };

                    for (double t = kStepSeconds; t < kOrbitSeconds; t += kStepSeconds)
                    {
                        const double angle = 360.0 * t / kOrbitSeconds;
                        actions.push_back({ kLeadIn + Seconds(t, fs), "move", [channel, angle]()
                                            {
                                                if (channel->Valid())
                                                    channel->SetLocation(LocationAt(angle));
                                            } });
                    }

                    return actions;
                };

                Capture capture;
                if (!RenderDeterministic(context, SettingsFor(layout, point, playEnd + Seconds(0.1, fs)), factory, "", out, capture))
                    return;

                const std::uint64_t begin = OnsetFrame(capture, kLeadIn) + Seconds(0.25, fs);
                const std::uint64_t end = playEnd - Seconds(0.05, fs);
                if (!RequireAnySignal(out, capture, begin, end))
                    return;

                // Total power in 20 ms windows: constant-power panning keeps it flat all the way round.
                const std::uint64_t window = Seconds(0.02, fs);
                std::vector<double> totals;
                for (std::uint64_t at = begin; at + window <= end; at += window)
                {
                    const std::vector<double> powers = ChannelPowers(capture, at, at + window);
                    double total = 0.0;
                    for (const double p : powers)
                        total += p;
                    totals.push_back(total);
                }

                const double median = MedianFinite(totals);
                double deviation = 0.0;
                for (const double total : totals)
                    deviation = std::max(deviation, std::abs(10.0 * std::log10(total / median)));

                out.Add("orbit.powerDeviationDb", deviation, "dB", Better::Lower, Targets::kMaxOrbitPowerDeviationDb);
                AddClickMetrics(out, capture, 4000.0, begin, end);
                AddPanIntegrityMetrics(out, capture, begin, end);
                out.capture = std::move(capture);
            }
        };

        class SpreadSweepScenario final : public Scenario
        {
        public:
            /// Each spread value is held this long; the parameter reaches the mixer a couple of frames late and ramps in, so
            /// the measurement skips the first kSettleSeconds of every hold.
            static constexpr double kHoldSeconds = 0.3;
            static constexpr double kSettleSeconds = 0.12;

            [[nodiscard]] std::string Id() const override
            {
                return "S2";
            }

            [[nodiscard]] std::string Title() const override
            {
                return "Spread sweep";
            }

            [[nodiscard]] std::uint32_t Dimensions() const override
            {
                return 0;
            }

            [[nodiscard]] std::vector<std::string> Variants() const override
            {
                return { "stereo", "surround51" };
            }

            /// The swept spread values, degrees: the stereo source's rise to full width (0 - 60), then on to 360.
            static const std::vector<double>& Spreads()
            {
                static const std::vector<double> spreads{ 0.0, 10.0, 20.0, 30.0, 40.0, 50.0, 60.0, 90.0, 180.0, 270.0, 360.0 };
                return spreads;
            }

            /// Weight of either channel of a stereo source at @p spread: the fold-down coefficient at 0, 1 at 60 and above.
            static double StereoWeight(double spread)
            {
                constexpr double k = 0.70710678118654752;
                return k + (1.0 - k) * std::min(spread, 60.0) / 60.0;
            }

            void Measure(const RunContext& context, const GridPoint& point, const std::string& variant, Measurement& out) const override
            {
                const double fs = point.outputRate;
                const OutputLayout& layout = FindLayout(variant);
                const StimulusSpec* noise = FindStimulus("pink_stereo_independent_48000");
                const std::uint64_t hold = Seconds(kHoldSeconds, fs);
                const std::uint64_t playEnd = kLeadIn + hold * Spreads().size();

                const ActionFactory factory = [hold]()
                {
                    std::vector<TimedAction> actions{ SceneSetup(),
                                                      { 0, "spread", []()
                                                        {
                                                            amEngine->GetRtpcHandle(kSpreadRtpcName)->SetValue(0.0);
                                                        } },
                                                      { kLeadIn, "play", []()
                                                        {
                                                            AM_UNUSED(amEngine->Play(amEngine->GetSoundHandle(kSpreadSweepSoundName), LocationAt(0.0)));
                                                        } } };

                    for (std::size_t k = 1; k < Spreads().size(); ++k)
                    {
                        const double spread = Spreads()[k];
                        actions.push_back({ kLeadIn + hold * k, "spread", [spread]()
                                            {
                                                amEngine->GetRtpcHandle(kSpreadRtpcName)->SetValue(spread);
                                            } });
                    }

                    return actions;
                };

                Capture capture;
                if (!RenderDeterministic(context, SettingsFor(layout, point, playEnd), factory, "", out, capture))
                    return;

                const std::uint64_t onset = OnsetFrame(capture, kLeadIn);
                const Signal left = RenderStimulusChannel(*noise, 0);
                const Signal right = RenderStimulusChannel(*noise, 1);

                // Windows of 0.18 s of white noise: the cross-term of the two channels is about 0.01 of their power.
                double worst = 0.0;
                for (std::size_t k = 0; k < Spreads().size(); ++k)
                {
                    const std::uint64_t at = onset + hold * k + Seconds(kSettleSeconds, fs);
                    const std::uint64_t window = hold - Seconds(kSettleSeconds, fs) - Seconds(0.02, fs);
                    const std::size_t source = static_cast<std::size_t>(at - onset);
                    if (source + window > left.size() || at + window > playEnd)
                        break;

                    const double sourcePower = std::pow(Rms(std::span<const double>(left.data() + source, window)), 2.0) +
                        std::pow(Rms(std::span<const double>(right.data() + source, window)), 2.0);

                    double outputPower = 0.0;
                    for (const double p : ChannelPowers(capture, at, at + window))
                        outputPower += p;

                    const double expected = std::pow(StereoWeight(Spreads()[k]), 2.0) * sourcePower;
                    worst = std::max(worst, std::abs(10.0 * std::log10(outputPower / expected)));
                }

                out.Add("spread.powerErrorDb", worst, "dB", Better::Lower, Targets::kMaxSpreadPowerErrorDb);
                AddPanIntegrityMetrics(out, capture, onset, playEnd);
                out.capture = std::move(capture);
            }
        };
    } // namespace

    std::unique_ptr<Scenario> MakePanLawScenario()
    {
        return std::make_unique<PanLawScenario>();
    }

    std::unique_ptr<Scenario> MakeOrbitScenario()
    {
        return std::make_unique<OrbitScenario>();
    }

    std::unique_ptr<Scenario> MakeSpreadSweepScenario()
    {
        return std::make_unique<SpreadSweepScenario>();
    }
} // namespace SparkyStudios::Audio::Amplitude::Fidelity
