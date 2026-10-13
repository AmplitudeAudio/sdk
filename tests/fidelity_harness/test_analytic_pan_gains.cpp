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
#include <limits>
#include <vector>

#include <Fidelity/Scenarios/Common.h>

#include "PureUnitTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude::Fidelity;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(PureUnitTestCase, fidelity_harness, analytic_pan_gains)
    {
    public:
        void Run() override
        {
            constexpr double k = 0.70710678118654752;
            constexpr double kRadians = 3.14159265358979323846 / 180.0;
            const double lfe = std::numeric_limits<double>::quiet_NaN();
            const std::vector<double> stereo{ 30.0, -30.0 };
            const std::vector<double> s51{ 30.0, -30.0, 0.0, lfe, 110.0, -110.0 };
            const auto isClose = [](const std::vector<double>& gains, std::initializer_list<double> expected)
            {
                std::size_t i = 0;
                bool ok = true;
                for (const double e : expected)
                    ok &= std::abs(gains[i++] - e) < 1e-9;
                return ok;
            };

            AM_EXPECT(isClose(AnalyticPanGains(0.0, stereo), { k, k }));
            AM_EXPECT(isClose(AnalyticPanGains(30.0, stereo), { 1.0, 0.0 }));
            AM_EXPECT(isClose(AnalyticPanGains(90.0, stereo), { 1.0, 0.0 }));
            AM_EXPECT(isClose(AnalyticPanGains(180.0, stereo), { k, k }));
            AM_EXPECT(isClose(AnalyticPanGains(0.0, s51), { 0.0, 0.0, 1.0, 0.0, 0.0, 0.0 }));
            AM_EXPECT(isClose(AnalyticPanGains(180.0, s51), { 0.0, 0.0, 0.0, 0.0, k, k }));

            // Between L (30) and Ls (110), the tangent law normalised for power.
            const std::vector<double> g = AnalyticPanGains(90.0, s51);
            AM_EXPECT(std::abs(g[0] * g[0] + g[4] * g[4] - 1.0) < 1e-12);
            AM_EXPECT(std::abs(g[4] / g[0] - std::sin(60.0 * kRadians) / std::sin(20.0 * kRadians)) < 1e-9);
        }
    };

    AM_REGISTER_TEST(fidelity_harness, analytic_pan_gains);
} // namespace SparkyStudios::Audio::Amplitude::Tests
