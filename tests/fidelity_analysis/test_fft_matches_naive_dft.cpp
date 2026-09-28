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

#include <random>

#include <Fidelity/Analysis/Fft.h>

#include "PureUnitTestCase.h"
#include "TestRegistry.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    using namespace SparkyStudios::Audio::Amplitude::Fidelity;

    AM_TEST_CASE(PureUnitTestCase, fidelity_analysis, fft_matches_naive_dft)
    {
    public:
        void Run() override
        {
            std::mt19937_64 rng(42);
            const auto next = [&rng]()
            {
                return static_cast<double>(rng() >> 11) * 0x1.0p-53 * 2.0 - 1.0;
            };

            for (std::size_t n = 2; n <= 4096; n <<= 1)
            {
                std::vector<Complex> input(n);
                for (auto& value : input)
                    value = Complex(next(), next());

                for (const bool inverse : { false, true })
                {
                    std::vector<Complex> fast = input;
                    Fft(fast, inverse);
                    const std::vector<Complex> slow = NaiveDft(input, inverse);

                    double maxError = 0.0;
                    double maxReference = 0.0;
                    for (std::size_t i = 0; i < n; ++i)
                    {
                        maxError = std::max(maxError, std::abs(fast[i] - slow[i]));
                        maxReference = std::max(maxReference, std::abs(slow[i]));
                    }

                    AM_EXPECT(maxError <= 1e-9 * maxReference);
                }
            }
        }
    };

    AM_REGISTER_TEST(fidelity_analysis, fft_matches_naive_dft);
} // namespace SparkyStudios::Audio::Amplitude::Tests
