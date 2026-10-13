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

#include <Fidelity/Analysis/Fft.h>
#include <Fidelity/Analysis/Window.h>

#include "PureUnitTestCase.h"
#include "TestRegistry.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    using namespace SparkyStudios::Audio::Amplitude::Fidelity;

    AM_TEST_CASE(PureUnitTestCase, fidelity_analysis, blackman_harris_window)
    {
    public:
        void Run() override
        {
            constexpr std::size_t n = 4096;
            const std::vector<double> window = BlackmanHarris7(n);

            AM_EXPECT_EQ(window.size(), n);
            AM_EXPECT(std::abs(window[0]) < 1e-6);
            AM_EXPECT(std::abs(window[n / 2] - 1.0) < 1e-9);
            AM_EXPECT(std::abs(CoherentGain(window) - 0.27105140069342) < 1e-12);
            AM_EXPECT(EquivalentNoiseBandwidth(window) > 2.0 && EquivalentNoiseBandwidth(window) < 3.5);

            // A tone halfway between bins leaks less than -150 dB ten bins away.
            std::vector<Complex> data(n);
            for (std::size_t i = 0; i < n; ++i)
                data[i] = window[i] * std::cos(2.0 * std::numbers::pi * 100.5 * static_cast<double>(i) / static_cast<double>(n));
            Fft(data, false);

            double peak = 0.0;
            for (std::size_t k = 0; k < n / 2; ++k)
                peak = std::max(peak, std::norm(data[k]));

            double worst = 0.0;
            for (std::size_t k = 0; k < n / 2; ++k)
                if (std::abs(static_cast<double>(k) - 100.5) >= 10.0)
                    worst = std::max(worst, std::norm(data[k]) / peak);

            AM_EXPECT(worst <= 1e-15);
        }
    };

    AM_REGISTER_TEST(fidelity_analysis, blackman_harris_window);
} // namespace SparkyStudios::Audio::Amplitude::Tests
