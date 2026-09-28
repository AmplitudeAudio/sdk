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

#include <Fidelity/Analysis/Window.h>

#include <array>
#include <cmath>
#include <numbers>

namespace SparkyStudios::Audio::Amplitude::Fidelity
{
    std::vector<double> BlackmanHarris7(std::size_t n)
    {
        static constexpr std::array<double, 7> kCoefficients = {
            0.27105140069342, 0.43329793923448, 0.21812299954311, 0.06592544638803, 0.01081174209837, 0.00077658482522, 0.00001388721735,
        };

        std::vector<double> window(n, 0.0);
        for (std::size_t k = 0; k < n; ++k)
        {
            double value = 0.0;
            for (std::size_t m = 0; m < kCoefficients.size(); ++m)
            {
                const double sign = m % 2 == 0 ? 1.0 : -1.0;
                value += sign * kCoefficients[m] * std::cos(2.0 * std::numbers::pi * static_cast<double>(m * k) / static_cast<double>(n));
            }

            window[k] = value;
        }

        return window;
    }

    double CoherentGain(const std::vector<double>& window)
    {
        if (window.empty())
            return 0.0;

        double sum = 0.0;
        for (const double w : window)
            sum += w;

        return sum / static_cast<double>(window.size());
    }

    double EquivalentNoiseBandwidth(const std::vector<double>& window)
    {
        double sum = 0.0;
        double sumSquares = 0.0;
        for (const double w : window)
        {
            sum += w;
            sumSquares += w * w;
        }

        return sum == 0.0 ? 0.0 : static_cast<double>(window.size()) * sumSquares / (sum * sum);
    }
} // namespace SparkyStudios::Audio::Amplitude::Fidelity
