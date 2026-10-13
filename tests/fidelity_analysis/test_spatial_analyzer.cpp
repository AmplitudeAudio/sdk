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

#include <Fidelity/Analysis/Spatial.h>
#include <Fidelity/Signal.h>

#include "PureUnitTestCase.h"
#include "TestRegistry.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    using namespace SparkyStudios::Audio::Amplitude::Fidelity;

    AM_TEST_CASE(PureUnitTestCase, fidelity_analysis, spatial_analyzer)
    {
    public:
        void Run() override
        {
            constexpr double fs = 48000.0;
            constexpr std::size_t n = 48000;

            Random random(5);
            Signal noise(n);
            for (auto& v : noise)
                v = 0.25 * random.Symmetric();

            // Right = left delayed by 12 samples.
            {
                Signal right(n, 0.0);
                for (std::size_t i = 12; i < n; ++i)
                    right[i] = noise[i - 12];

                SpatialOptions options;
                options.begin = 480;
                const SpatialResult r = AnalyzeSpatial(noise, right, fs, options);
                bool allClose = !r.itdSamples.empty();
                for (const double itd : r.itdSamples)
                    allClose = allClose && std::abs(itd - 12.0) <= 0.1;

                AM_EXPECT(allClose);
                AM_EXPECT(r.largestItdStepSamples <= 0.2);
            }

            // ILD ramps 0 -> 6 dB over 100 windows with a 1 dB jump at window 50.
            {
                Signal left(n);
                for (std::size_t i = 0; i < n; ++i)
                {
                    const std::size_t window = i / 480;
                    const double ild = 6.0 * static_cast<double>(window) / 99.0 + (window >= 50 ? 1.0 : 0.0);
                    left[i] = noise[i] * AmplitudeFromDb(ild);
                }

                const SpatialResult r = AnalyzeSpatial(left, noise, fs, SpatialOptions{});
                AM_EXPECT(r.largestIldStepDb >= 0.95 && r.largestIldStepDb <= 1.15);
                AM_EXPECT(r.largestIldStepWindow >= 49 && r.largestIldStepWindow <= 51);
            }
        }
    };

    AM_REGISTER_TEST(fidelity_analysis, spatial_analyzer);
} // namespace SparkyStudios::Audio::Amplitude::Tests
