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
#include <random>

#include <Fidelity/Analysis/Fft.h>

#include "PureUnitTestCase.h"
#include "TestRegistry.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    using namespace SparkyStudios::Audio::Amplitude::Fidelity;

    AM_TEST_CASE(PureUnitTestCase, fidelity_analysis, fft_handles_large_sizes)
    {
    public:
        void Run() override
        {
            constexpr std::size_t n = 65536;

            // Round trip.
            std::mt19937_64 rng(7);
            std::vector<Complex> input(n);
            for (auto& value : input)
                value = Complex(static_cast<double>(rng() >> 11) * 0x1.0p-53 - 0.5, 0.0);

            std::vector<Complex> data = input;
            Fft(data, false);
            Fft(data, true);

            double maxError = 0.0;
            for (std::size_t i = 0; i < n; ++i)
                maxError = std::max(maxError, std::abs(data[i] - input[i]));
            AM_EXPECT(maxError <= 1e-12);

            // A unit cosine on bin 1000 lands on bins 1000 and n - 1000 with magnitude n / 2.
            std::vector<Complex> tone(n);
            for (std::size_t i = 0; i < n; ++i)
                tone[i] = std::cos(2.0 * std::numbers::pi * 1000.0 * static_cast<double>(i) / static_cast<double>(n));
            Fft(tone, false);

            const double half = static_cast<double>(n) / 2.0;
            AM_EXPECT(std::abs(tone[1000] - Complex(half, 0.0)) <= 1e-6 * half);
            AM_EXPECT(std::abs(tone[n - 1000] - Complex(half, 0.0)) <= 1e-6 * half);

            double leakage = 0.0;
            for (std::size_t k = 0; k < n; ++k)
                if (k != 1000 && k != n - 1000)
                    leakage = std::max(leakage, std::abs(tone[k]));
            AM_EXPECT(leakage <= 1e-9 * static_cast<double>(n));
        }
    };

    AM_REGISTER_TEST(fidelity_analysis, fft_handles_large_sizes);
} // namespace SparkyStudios::Audio::Amplitude::Tests
