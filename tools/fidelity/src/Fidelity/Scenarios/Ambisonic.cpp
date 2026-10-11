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
#include <limits>
#include <memory>
#include <string>
#include <vector>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Fidelity/AssetGenerator.h>
#include <Fidelity/Scenarios/Common.h>
#include <Fidelity/Scenarios/Playback.h>
#include <Fidelity/Targets.h>

namespace SparkyStudios::Audio::Amplitude::Fidelity
{
    namespace
    {
        constexpr std::size_t kLfeChannel = 3;
        constexpr std::size_t kLeftSurroundChannel = 4;
        constexpr std::size_t kRightSurroundChannel = 5;

        class AmbisonicOrbitScenario final : public Scenario
        {
        public:
            static constexpr std::size_t kSteps = 36;
            static constexpr double kStepDegrees = 10.0;
            static constexpr double kHoldSeconds = 0.3;
            static constexpr double kSettleSeconds = 0.1;
            static constexpr double kTailSeconds = 0.02;

            [[nodiscard]] std::string Id() const override
            {
                return "A1";
            }

            [[nodiscard]] std::string Title() const override
            {
                return "Ambisonic orbit";
            }

            /// The ambisonic config exists at block 1024, 48 kHz only: the quick grid point.
            [[nodiscard]] std::uint32_t Dimensions() const override
            {
                return 0;
            }

            [[nodiscard]] std::vector<std::string> Variants() const override
            {
                return { "surround51" };
            }

            void Measure(const RunContext& context, const GridPoint& point, const std::string& variant, Measurement& out) const override
            {
                AM_UNUSED(variant);

                const double fs = point.outputRate;
                const std::uint64_t hold = Seconds(kHoldSeconds, fs);
                const std::uint64_t playEnd = kLeadIn + hold * kSteps;

                RenderSettings settings = IsolatedSettings(point, playEnd + Seconds(0.1, fs));
                settings.configFile = ConfigName(
                    ConfigKind(ConfigVariant{ point.blockSize, point.outputRate, 6, true }), point.blockSize, point.outputRate,
                    ".config.amconfig");

                const ActionFactory factory = [hold]()
                {
                    auto channel = std::make_shared<Channel>();
                    std::vector<TimedAction> actions{ SceneSetup(),
                                                      { kLeadIn, "play", [channel]()
                                                        {
                                                            *channel = amEngine->Play(amEngine->GetSoundHandle(kHrtfNoiseName), LocationAt(0.0));
                                                        } } };

                    for (std::size_t k = 1; k < kSteps; ++k)
                    {
                        const double angle = kStepDegrees * static_cast<double>(k);
                        actions.push_back({ kLeadIn + hold * k, "move", [channel, angle]()
                                            {
                                                if (channel->Valid())
                                                    channel->SetLocation(LocationAt(angle));
                                            } });
                    }

                    return actions;
                };

                Capture capture;
                if (!RenderDeterministic(context, settings, factory, "", out, capture))
                    return;

                const std::uint64_t onset = OnsetFrame(capture, kLeadIn);
                std::vector<std::vector<double>> steps;
                for (std::size_t k = 0; k < kSteps; ++k)
                {
                    const std::uint64_t begin = onset + hold * k + Seconds(kSettleSeconds, fs);
                    const std::uint64_t end = onset + hold * (k + 1) - Seconds(kTailSeconds, fs);
                    if (end > capture.channels[0].size())
                    {
                        out.error = "capture ends before the orbit does";
                        out.capture = std::move(capture);
                        return;
                    }

                    steps.push_back(ChannelPowers(capture, begin, end));
                }

                double maxTotal = 0.0;
                double minTotal = std::numeric_limits<double>::max();
                for (const std::vector<double>& powers : steps)
                {
                    double total = 0.0;
                    for (std::size_t c = 0; c < powers.size(); ++c)
                        if (c != kLfeChannel)
                            total += powers[c];

                    maxTotal = std::max(maxTotal, total);
                    minTotal = std::min(minTotal, total);
                }

                if (maxTotal <= 1e-8)
                {
                    out.error = "no signal on any channel";
                    out.capture = std::move(capture);
                    return;
                }

                // Step 3 is 30 degrees: front left.
                const std::vector<double>& frontLeft = steps[3];
                const double leakDb = 10.0 * std::log10(std::max(frontLeft[kRightSurroundChannel], 1e-20) /
                                                        std::max(frontLeft[kLeftSurroundChannel], 1e-20));

                float lfePeak = 0.0f;
                for (const float sample : capture.channels[kLfeChannel])
                    lfePeak = std::max(lfePeak, std::abs(sample));

                out.Add("ambisonic.orbitSpreadDb", 10.0 * std::log10(maxTotal / std::max(minTotal, 1e-20)), "dB", Better::Lower,
                        Targets::kMaxAmbisonicOrbitSpreadDb);
                out.Add("ambisonic.frontLeftRearLeakDb", leakDb, "dB", Better::Lower, Targets::kMaxAmbisonicFrontLeftRearLeakDb);
                out.Add("ambisonic.lfeDbfs", lfePeak > 0.0f ? DbFromAmplitude(lfePeak) : -200.0, "dBFS", Better::Lower,
                        Targets::kMaxAmbisonicLfeDbfs);
                AddPanIntegrityMetrics(out, capture, onset, playEnd);
                out.capture = std::move(capture);
            }
        };
    } // namespace

    std::unique_ptr<Scenario> MakeAmbisonicOrbitScenario()
    {
        return std::make_unique<AmbisonicOrbitScenario>();
    }
} // namespace SparkyStudios::Audio::Amplitude::Fidelity
