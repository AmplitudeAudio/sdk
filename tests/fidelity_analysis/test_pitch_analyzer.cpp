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

#include <numbers>

#include <Fidelity/Analysis/Pitch.h>
#include <Fidelity/Signal.h>
#include <Fidelity/Targets.h>

#include "PureUnitTestCase.h"
#include "TestRegistry.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    using namespace SparkyStudios::Audio::Amplitude::Fidelity;

    namespace
    {
        constexpr double kRate = 48000.0;
        constexpr std::size_t kLength = 96000;

        // Phase-accumulated sine following frequency(i), with an optional phase offset from `jumpAt` on.
        Signal Oscillate(const Signal& frequency, std::size_t jumpAt = SIZE_MAX, double jump = 0.0)
        {
            Signal x(frequency.size());
            double phase = 0.0;
            for (std::size_t i = 0; i < frequency.size(); ++i)
            {
                phase += 2.0 * std::numbers::pi * frequency[i] / kRate;
                x[i] = 0.5 * std::sin(phase + (i >= jumpAt ? jump : 0.0));
            }

            return x;
        }
    } // namespace

    AM_TEST_CASE(PureUnitTestCase, fidelity_analysis, pitch_analyzer)
    {
    public:
        void Run() override
        {
            PitchOptions base;
            base.begin = kLength / 10;
            base.end = kLength - kLength / 10;

            // A continuous octave glide against its own curve: under the floors, no grid, no phase jump.
            {
                Signal frequency(kLength);
                for (std::size_t i = 0; i < kLength; ++i)
                    frequency[i] = 997.0 * std::exp2(static_cast<double>(i) / static_cast<double>(kLength));

                PitchOptions options = base;
                options.expectedHz = frequency;
                const PitchResult r = AnalyzePitch(Oscillate(frequency), kRate, options);
                AM_EXPECT(r.rmsDeviationCents <= Floors::kPitchRmsCents);
                AM_EXPECT(r.largestStepCents <= Floors::kPitchStepCents);
                AM_EXPECT(r.quantizationGridCents == 0.0);
                AM_EXPECT_EQ(r.phaseJumpCount, 0);
            }

            // The same glide quantized to 2-cent steps every 1024 samples.
            {
                Signal stepped(kLength);
                Signal smooth(kLength);
                for (std::size_t i = 0; i < kLength; ++i)
                {
                    stepped[i] = 997.0 * std::exp2(2.0 * static_cast<double>(i / 1024) / 1200.0);
                    smooth[i] = 997.0 * std::exp2(2.0 * static_cast<double>(i) / 1024.0 / 1200.0);
                }

                PitchOptions options = base;
                options.expectedHz = smooth;
                const PitchResult r = AnalyzePitch(Oscillate(stepped), kRate, options);
                AM_EXPECT(std::abs(r.quantizationGridCents - 2.0) <= 0.1);
                AM_EXPECT(r.largestStepCents >= 1.5 && r.largestStepCents <= 2.3);
            }

            // A 0.1 rad phase jump is one event at the right sample.
            {
                const Signal frequency(kLength, 997.0);
                PitchOptions options = base;
                options.constantExpectedHz = 997.0;
                const PitchResult r = AnalyzePitch(Oscillate(frequency, 48000, 0.1), kRate, options);
                AM_EXPECT_EQ(r.phaseJumpCount, 1);
                AM_EXPECT(!r.phaseJumps.empty() && r.phaseJumps[0] + 2 >= 48000 && r.phaseJumps[0] <= 48002);
            }
        }
    };

    AM_REGISTER_TEST(fidelity_analysis, pitch_analyzer);
} // namespace SparkyStudios::Audio::Amplitude::Tests
