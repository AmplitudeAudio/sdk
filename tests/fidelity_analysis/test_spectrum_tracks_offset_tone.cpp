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

#include "PureUnitTestCase.h"
#include "TestRegistry.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    using namespace SparkyStudios::Audio::Amplitude::Fidelity;

    // The resampled output of the engine is not exactly at the nominal frequency: the analyzer must find the tone,
    // report how far it moved, and keep the tone's own lobe out of the spur measurement.
    AM_TEST_CASE(PureUnitTestCase, fidelity_analysis, spectrum_tracks_offset_tone)
    {
    public:
        void Run() override
        {
            constexpr double fs = 48000.0;
            constexpr std::size_t n = 96000;

            // A 15 kHz tone that came out 12.7 Hz flat (-1.47 cents).
            {
                const double actual = 15000.0 - 12.7;
                SpectrumOptions options;
                options.fundamentalHz = 15000.0;
                const SpectrumResult r = AnalyzeSpectrum(MakeSine(n, fs, actual, 0.5), fs, options);
                AM_EXPECT(std::abs(r.fundamentalDbfs - DbFromAmplitude(0.5)) <= 0.05);
                AM_EXPECT(std::abs(r.measuredFundamentalHz - actual) <= 0.05);
                AM_EXPECT(std::abs(r.frequencyErrorCents - CentsBetween(actual, 15000.0)) <= 0.05);
                AM_EXPECT(r.thdnDb <= -120.0);
                AM_EXPECT(r.worstSpurDbc <= -120.0);
            }

            // A -100 dBc spur 12 bins above the tone, next to the tone's masked main lobe.
            {
                const Signal tone = MakeSine(n, fs, 997.0, 0.5);
                const double binHz = fs / 32768.0;
                const Signal spur = MakeSine(n, fs, 997.0 + 12.0 * binHz, 0.5 * 1e-5);
                Signal x(n);
                for (std::size_t i = 0; i < n; ++i)
                    x[i] = tone[i] + spur[i];

                const SpectrumResult r = AnalyzeSpectrum(x, fs, SpectrumOptions{});
                AM_EXPECT(std::abs(r.worstSpurDbc - (-100.0)) <= 1.0);
                AM_EXPECT(std::abs(r.frequencyErrorCents) <= 0.05);
            }
        }
    };

    AM_REGISTER_TEST(fidelity_analysis, spectrum_tracks_offset_tone);
} // namespace SparkyStudios::Audio::Amplitude::Tests
