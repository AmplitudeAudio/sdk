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

#include <Fidelity/Analysis/Null.h>
#include <Fidelity/Signal.h>
#include <Fidelity/Targets.h>

#include "PureUnitTestCase.h"
#include "TestRegistry.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    using namespace SparkyStudios::Audio::Amplitude::Fidelity;

    AM_TEST_CASE(PureUnitTestCase, fidelity_analysis, null_analyzer)
    {
    public:
        void Run() override
        {
            constexpr std::size_t n = 48000;

            // Broadband b so the correlation peak is unambiguous; uniform noise with RMS 0.25 (-12 dBFS).
            Random random(3);
            Signal b(n);
            for (auto& v : b)
                v = random.Symmetric() * 0.25 * std::sqrt(3.0);

            // a = -1 dB copy of b delayed by 37 samples, plus -130 dBFS RMS noise.
            const double gain = AmplitudeFromDb(-1.0);
            const double noise = AmplitudeFromDb(-130.0) * std::sqrt(3.0);
            Signal a(n, 0.0);
            for (std::size_t i = 0; i < n; ++i)
                a[i] = (i >= 37 ? gain * b[i - 37] : 0.0) + random.Symmetric() * noise;

            NullOptions options;
            options.guard = 64;
            const NullResult delayed = AnalyzeNull(a, b, options);
            AM_EXPECT_EQ(delayed.lag, 37);
            AM_EXPECT(std::abs(delayed.gainDb - (-1.0)) <= 0.01);
            AM_EXPECT(std::abs(delayed.residualRmsDbfs - (-130.0)) <= 1.0);

            // Identical signals null completely.
            const NullResult same = AnalyzeNull(b, b, NullOptions{});
            AM_EXPECT_EQ(same.lag, 0);
            AM_EXPECT(same.residualPeakDbfs <= -300.0);

            // Float rounding sits below the analyzer floor.
            const Signal sine = MakeSine(n, 48000.0, 997.0, 0.5);
            const Signal rounded = ToSignal(ToFloats(sine));
            NullOptions exact;
            exact.maxLag = 0;
            exact.matchGain = false;
            AM_EXPECT(AnalyzeNull(sine, rounded, exact).residualPeakDbfs <= Floors::kNullDbfs);

            // Bit-exact comparison.
            std::vector<float> x = ToFloats(sine);
            std::vector<float> y = x;
            AM_EXPECT_EQ(CountDifferences(x, y), 0);
            y[100] = std::nextafter(y[100], 1.0f);
            AM_EXPECT_EQ(CountDifferences(x, y), 1);
            y.push_back(0.0f);
            AM_EXPECT_EQ(CountDifferences(x, y), 2);
        }
    };

    AM_REGISTER_TEST(fidelity_analysis, null_analyzer);
} // namespace SparkyStudios::Audio::Amplitude::Tests
