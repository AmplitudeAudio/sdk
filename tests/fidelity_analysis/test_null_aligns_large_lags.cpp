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

    // Two renders of one stimulus can start blocks apart (P6: 5155 vs 12323 samples). A tone correlates almost as well
    // one period away, so the lag comes from the onsets and the correlation only refines it.
    AM_TEST_CASE(PureUnitTestCase, fidelity_analysis, null_aligns_large_lags)
    {
    public:
        void Run() override
        {
            constexpr std::size_t length = 120000;
            Signal tone = MakeSine(96000, 48000.0, 997.0, 0.5);
            ApplyFades(tone, 960);

            const auto placed = [&](std::size_t at)
            {
                Signal x(length, 0.0);
                for (std::size_t i = 0; i < tone.size() && at + i < length; ++i)
                    x[at + i] = tone[i];

                return x;
            };

            const Signal a = placed(5155);
            const Signal b = placed(12323);

            const std::int64_t coarse = static_cast<std::int64_t>(OnsetSample(a, 1e-3)) - static_cast<std::int64_t>(OnsetSample(b, 1e-3));
            AM_EXPECT_EQ(coarse, -7168);

            NullOptions options;
            options.matchGain = false;
            options.lagCenter = coarse;
            options.maxLag = 64;
            const NullResult r = AnalyzeNull(a, b, options);
            AM_EXPECT_EQ(r.lag, -7168);
            AM_EXPECT(r.residualPeakDbfs <= Floors::kNullDbfs);

            AM_EXPECT_EQ(OnsetSample(Signal(100, 0.0), 1e-3), 100);
        }
    };

    AM_REGISTER_TEST(fidelity_analysis, null_aligns_large_lags);
} // namespace SparkyStudios::Audio::Amplitude::Tests
