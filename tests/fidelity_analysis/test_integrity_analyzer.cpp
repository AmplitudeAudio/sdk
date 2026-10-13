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

#include <limits>

#include <Fidelity/Analysis/Integrity.h>
#include <Fidelity/Signal.h>

#include "PureUnitTestCase.h"
#include "TestRegistry.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    using namespace SparkyStudios::Audio::Amplitude::Fidelity;

    AM_TEST_CASE(PureUnitTestCase, fidelity_analysis, integrity_analyzer)
    {
    public:
        void Run() override
        {
            std::vector<float> clean = ToFloats(MakeSine(48000, 48000.0, 997.0, 0.5));
            IntegrityOptions options;
            options.signalBegin = 0;
            options.signalEnd = clean.size();

            const IntegrityResult ok = AnalyzeIntegrity(clean, options);
            AM_EXPECT_EQ(ok.nanCount, 0);
            AM_EXPECT_EQ(ok.infCount, 0);
            AM_EXPECT_EQ(ok.denormalCount, 0);
            AM_EXPECT_EQ(ok.clippedCount, 0);
            AM_EXPECT_EQ(ok.dropoutCount, 0);

            std::vector<float> broken = clean;
            broken[10] = std::numeric_limits<float>::quiet_NaN();
            broken[20] = std::numeric_limits<float>::infinity();
            broken[30] = 1e-40f;
            broken[40] = 1.5f;
            for (std::size_t i = 1000; i < 1100; ++i)
                broken[i] = 0.0f;

            const IntegrityResult bad = AnalyzeIntegrity(broken, options);
            AM_EXPECT_EQ(bad.nanCount, 1);
            AM_EXPECT_EQ(bad.infCount, 1);
            AM_EXPECT_EQ(bad.denormalCount, 1);
            AM_EXPECT_EQ(bad.clippedCount, 1);
            AM_EXPECT_EQ(bad.dropoutCount, 1);
            AM_EXPECT_EQ(bad.longestDropout, 100);
            AM_EXPECT(bad.dropoutStarts.size() == 1 && bad.dropoutStarts[0] == 1000);

            // Without a signal region, silence is not a dropout.
            const IntegrityResult noRegion = AnalyzeIntegrity(std::vector<float>(4096, 0.0f), IntegrityOptions{});
            AM_EXPECT_EQ(noRegion.dropoutCount, 0);
        }
    };

    AM_REGISTER_TEST(fidelity_analysis, integrity_analyzer);
} // namespace SparkyStudios::Audio::Amplitude::Tests
