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

    AM_TEST_CASE(PureUnitTestCase, fidelity_analysis, chirp_source_time)
    {
    public:
        void Run() override
        {
            constexpr double fs = 48000.0;
            constexpr std::size_t n = 96000;
            constexpr double offset = 1234.5;
            const ChirpModel model{ 200.0, 450.0, 0.5 };

            Signal x(n);
            for (std::size_t i = 0; i < n; ++i)
            {
                const double tau = (static_cast<double>(i) + offset) / fs;
                x[i] = 0.5 * std::sin(2.0 * std::numbers::pi * std::fmod(ChirpCycles(model, tau), 1.0));
            }

            const Signal tau = EstimateChirpSourceTime(x, fs, model, 0.4);

            double worst = 0.0;
            for (std::size_t i = n / 10; i < n - n / 10; ++i)
                worst = std::max(worst, std::abs(tau[i] * fs - static_cast<double>(i) - offset));

            AM_EXPECT(worst <= Floors::kPositionSamples);
        }
    };

    AM_REGISTER_TEST(fidelity_analysis, chirp_source_time);
} // namespace SparkyStudios::Audio::Amplitude::Tests
