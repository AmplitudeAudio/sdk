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
#include <numbers>
#include <vector>

#include <Fidelity/Analysis/Sideband.h>

#include "PureUnitTestCase.h"
#include "TestRegistry.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    using namespace SparkyStudios::Audio::Amplitude::Fidelity;

    // A tone whose phase is modulated at a known rate and depth is the whole reference: the analyzer has to read
    // the sidebands such a tone puts at that rate, and stay off them when the modulation is gone.
    AM_TEST_CASE(PureUnitTestCase, fidelity_analysis, glide_sideband_analyzer)
    {
    public:
        void Run() override
        {
            constexpr double kPi = std::numbers::pi;
            constexpr double kRate = 48000.0;
            constexpr double kCarrier = 3000.0;
            constexpr double kBlockRate = kRate / 1024.0;

            const auto measure = [&](double depth)
            {
                constexpr std::size_t kFrames = 65536;
                Signal x(kFrames);
                Signal tracked(kFrames, kCarrier);
                for (std::size_t n = 0; n < kFrames; ++n)
                {
                    const double t = static_cast<double>(n) / kRate;
                    x[n] = 0.5 * std::sin(2.0 * kPi * kCarrier * t + depth * std::sin(2.0 * kPi * kBlockRate * t));
                }

                GlideSidebandOptions options;
                options.sampleRate = kRate;
                options.blockRateHz = kBlockRate;
                return AnalyzeGlideSidebands(x, tracked, options);
            };

            // A carrier with no modulation has no sidebands to find, and the reading says so rather than reporting
            // whatever the window's own leakage happens to be.
            const GlideSidebandResult clean = measure(0.0);
            AM_EXPECT(clean.carrierDbfs > -12.0);
            AM_EXPECT(clean.sidebandDbc < -90.0);

            // Half a radian of phase modulation puts most of its energy in the first sideband pair.
            const GlideSidebandResult modulated = measure(0.5);
            AM_EXPECT(modulated.sidebandDbc > clean.sidebandDbc + 40.0);

            // Phase modulation of depth b puts J_n(b)^2 of the power at the n-th sideband either side: for b = 0.5 the
            // first three pairs hold 0.1193 against 0.8808 in the carrier, -8.68 dBc.
            AM_EXPECT(std::abs(modulated.sidebandDbc - -8.68) < 0.5);
        }
    };

    AM_REGISTER_TEST(fidelity_analysis, glide_sideband_analyzer);
} // namespace SparkyStudios::Audio::Amplitude::Tests
