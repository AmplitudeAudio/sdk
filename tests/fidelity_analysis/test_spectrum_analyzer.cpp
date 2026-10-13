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

#include <Fidelity/Analysis/Spectrum.h>
#include <Fidelity/Signal.h>
#include <Fidelity/Targets.h>

#include "PureUnitTestCase.h"
#include "TestRegistry.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    using namespace SparkyStudios::Audio::Amplitude::Fidelity;

    AM_TEST_CASE(PureUnitTestCase, fidelity_analysis, spectrum_analyzer)
    {
    public:
        void Run() override
        {
            constexpr double fs = 48000.0;
            constexpr std::size_t n = 96000;
            const Signal fundamental = MakeSine(n, fs, 997.0, 0.5);

            // Harmonics at -80 and -90 dBc: THD = 10 log10(1e-8 + 1e-9).
            {
                const Signal h2 = MakeSine(n, fs, 1994.0, 0.5 * 1e-4, 0.3);
                const Signal h3 = MakeSine(n, fs, 2991.0, 0.5 * AmplitudeFromDb(-90.0), 1.1);
                Signal x(n);
                for (std::size_t i = 0; i < n; ++i)
                    x[i] = fundamental[i] + h2[i] + h3[i];

                const SpectrumResult r = AnalyzeSpectrum(x, fs, SpectrumOptions{});
                AM_EXPECT(std::abs(r.fundamentalDbfs - DbFromAmplitude(0.5)) <= 0.05);
                AM_EXPECT(std::abs(r.thdDb - 10.0 * std::log10(1e-8 + 1e-9)) <= 0.5);
            }

            // White noise at -90 dB relative to the tone power over the full band; THD+N counts 20 Hz..20 kHz of it.
            {
                Random random(11);
                const double noiseVariance = 0.125 * 1e-9;
                const double amplitude = std::sqrt(3.0 * noiseVariance);
                Signal x(n);
                for (std::size_t i = 0; i < n; ++i)
                    x[i] = fundamental[i] + amplitude * random.Symmetric();

                const SpectrumResult r = AnalyzeSpectrum(x, fs, SpectrumOptions{});
                const double expected = 10.0 * std::log10(1e-9 * (20000.0 - 20.0) / 24000.0);
                AM_EXPECT(std::abs(r.thdnDb - expected) <= 0.5);
                AM_EXPECT(std::abs(r.sinadDb + r.thdnDb) < 1e-12);
            }

            // A -100 dBc spur is found at its frequency.
            {
                const Signal spur = MakeSine(n, fs, 3300.0, 0.5 * 1e-5);
                Signal x(n);
                for (std::size_t i = 0; i < n; ++i)
                    x[i] = fundamental[i] + spur[i];

                const SpectrumResult r = AnalyzeSpectrum(x, fs, SpectrumOptions{});
                AM_EXPECT(std::abs(r.worstSpurDbc - (-100.0)) <= 1.0);
                AM_EXPECT(std::abs(r.worstSpurHz - 3300.0) <= 2.0);
            }

            // A float-rounded clean tone sits under the analyzer floor.
            {
                const SpectrumResult r = AnalyzeSpectrum(ToSignal(ToFloats(fundamental)), fs, SpectrumOptions{});
                AM_EXPECT(r.thdnDb <= Floors::kSpectrumDb);
            }
        }
    };

    AM_REGISTER_TEST(fidelity_analysis, spectrum_analyzer);
} // namespace SparkyStudios::Audio::Amplitude::Tests
