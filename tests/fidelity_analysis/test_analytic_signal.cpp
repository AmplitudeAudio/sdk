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

#include <Fidelity/Analysis/Analytic.h>
#include <Fidelity/Signal.h>

#include "PureUnitTestCase.h"
#include "TestRegistry.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    using namespace SparkyStudios::Audio::Amplitude::Fidelity;

    AM_TEST_CASE(PureUnitTestCase, fidelity_analysis, analytic_signal)
    {
    public:
        void Run() override
        {
            constexpr double fs = 48000.0;
            constexpr std::size_t n = 48000;
            const std::size_t begin = n / 10;
            const std::size_t end = n - n / 10;

            // Envelope of a steady tone.
            const Signal sine = MakeSine(n, fs, 997.0, 0.5);
            const Signal envelope = Envelope(sine);
            double worstDb = 0.0;
            for (std::size_t i = begin; i < end; ++i)
                worstDb = std::max(worstDb, std::abs(DbFromAmplitude(envelope[i]) - DbFromAmplitude(0.5)));
            AM_EXPECT(worstDb <= 0.001);

            // Instantaneous frequency from the unwrapped phase.
            const Signal phase = UnwrappedPhase(sine);
            double worstHz = 0.0;
            for (std::size_t i = begin; i < end; ++i)
            {
                const double frequency = (phase[i + 1] - phase[i - 1]) * fs / (4.0 * std::numbers::pi);
                worstHz = std::max(worstHz, std::abs(frequency - 997.0));
            }
            AM_EXPECT(worstHz <= 0.01);

            // Slow amplitude modulation is tracked.
            Signal modulated(n);
            for (std::size_t i = 0; i < n; ++i)
                modulated[i] = sine[i] * (1.0 + 0.5 * std::sin(2.0 * std::numbers::pi * 2.0 * static_cast<double>(i) / fs));
            const Signal modulatedEnvelope = Envelope(modulated);
            double worstRelative = 0.0;
            for (std::size_t i = begin; i < end; ++i)
            {
                const double expected = 0.5 * (1.0 + 0.5 * std::sin(2.0 * std::numbers::pi * 2.0 * static_cast<double>(i) / fs));
                worstRelative = std::max(worstRelative, std::abs(modulatedEnvelope[i] / expected - 1.0));
            }
            AM_EXPECT(worstRelative <= 1e-3);
        }
    };

    AM_REGISTER_TEST(fidelity_analysis, analytic_signal);
} // namespace SparkyStudios::Audio::Amplitude::Tests
