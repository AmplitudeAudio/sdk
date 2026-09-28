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

#include <Fidelity/Signal.h>

#include "PureUnitTestCase.h"
#include "TestRegistry.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    using namespace SparkyStudios::Audio::Amplitude::Fidelity;

    AM_TEST_CASE(PureUnitTestCase, fidelity_analysis, signal_helpers)
    {
    public:
        void Run() override
        {
            AM_EXPECT(std::abs(DbFromAmplitude(0.5) - (-6.0205999)) < 1e-6);
            AM_EXPECT(DbFromAmplitude(0.0) == -400.0);
            AM_EXPECT(std::abs(DbFromPower(0.01) - (-20.0)) < 1e-12);
            AM_EXPECT(std::abs(AmplitudeFromDb(-20.0) - 0.1) < 1e-12);
            AM_EXPECT(std::abs(CentsBetween(880.0, 440.0) - 1200.0) < 1e-9);

            const std::vector<double> square = { 1.0, -1.0, 1.0, -1.0 };
            AM_EXPECT(std::abs(Rms(square) - 1.0) < 1e-12);
            AM_EXPECT(Peak(std::vector<double>{ 0.25, -0.75, 0.5 }) == 0.75);

            AM_EXPECT(FadeGain(0, 1000, 100) == 0.0);
            AM_EXPECT(std::abs(FadeGain(50, 1000, 100) - 0.5) < 1e-12);
            AM_EXPECT(FadeGain(100, 1000, 100) == 1.0);
            AM_EXPECT(std::abs(FadeGain(949, 1000, 100) - 0.5) < 1e-12);
            AM_EXPECT(FadeGain(999, 1000, 100) == 0.0);

            Signal ones(1000, 1.0);
            ApplyFades(ones, 100);
            AM_EXPECT(ones[0] == 0.0 && ones[500] == 1.0 && ones[999] == 0.0);

            const Signal average = MovingAverage(std::vector<double>(64, 3.0), 9);
            AM_EXPECT(std::abs(average.front() - 3.0) < 1e-12 && std::abs(average.back() - 3.0) < 1e-12);

            const Signal sine = MakeSine(48000, 48000.0, 1000.0, 0.5);
            AM_EXPECT(std::abs(sine[12] - 0.5) < 1e-12); // a quarter period of 1 kHz at 48 kHz
            AM_EXPECT(ToFloats(sine).size() == sine.size());

            Random a(99);
            Random b(99);
            bool same = true;
            bool inRange = true;
            for (int i = 0; i < 1000; ++i)
            {
                const double u = a.Uniform();
                same = same && u == b.Uniform();
                inRange = inRange && u >= 0.0 && u < 1.0;
            }
            AM_EXPECT(same);
            AM_EXPECT(inRange);
        }
    };

    AM_REGISTER_TEST(fidelity_analysis, signal_helpers);
} // namespace SparkyStudios::Audio::Amplitude::Tests
