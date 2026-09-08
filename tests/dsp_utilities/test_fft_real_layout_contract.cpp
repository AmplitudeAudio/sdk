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

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include "DSPTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(DSPTestCase, dsp_utilities, fft_real_layout_contract)
    {
    public:
        void Run() override
        {
            constexpr AmSize size = 256;
            constexpr AmSize bins = size / 2 + 1;

            FFT fft;
            fft.Initialize(size);

            // DC of 0.5 plus a Nyquist component of 0.25: both bins must be exactly
            // representable and purely real in the split-complex layout.
            AudioBuffer input(size, 1);
            for (AmSize i = 0; i < size; ++i)
                input[0][i] = 0.5f + 0.25f * ((i % 2 == 0) ? 1.0f : -1.0f);

            SplitComplex spectrum;
            fft.Forward(input[0].begin(), spectrum);

            AM_EXPECT(spectrum.GetSize() == bins);

            // DC bin: purely real, amplitude = N * dc.
            AM_EXPECT(std::abs(spectrum.re()[0] - static_cast<AmReal32>(size) * 0.5f) < 0.5f);
            AM_EXPECT(std::abs(spectrum.im()[0]) < 1e-3f);

            // Nyquist bin: purely real, amplitude = N * amplitude.
            AM_EXPECT(std::abs(spectrum.re()[size / 2] - static_cast<AmReal32>(size) * 0.25f) < 0.5f);
            AM_EXPECT(std::abs(spectrum.im()[size / 2]) < 1e-3f);

            // Round trip through the layout must reconstruct the input.
            AudioBuffer output(size, 1);
            fft.Backward(output[0].begin(), spectrum);
            for (AmSize i = 0; i < size; ++i)
                AM_EXPECT(std::abs(output[0][i] - input[0][i]) < 1e-3f);

            // Add a mid-bin sinusoid (bin 10 of 256: exactly 10 cycles per
            // window) so the mid-bin unpack path is covered explicitly.
            for (AmSize i = 0; i < size; ++i)
                input[0][i] += 0.125f * std::sin(2.0f * AM_PI32 * 10.0f * static_cast<AmReal32>(i) / static_cast<AmReal32>(size));

            fft.Forward(input[0].begin(), spectrum);

            // An integer number of cycles lands entirely in one bin.
            const AmReal32 binMag = std::sqrt(spectrum.re()[10] * spectrum.re()[10] + spectrum.im()[10] * spectrum.im()[10]);
            AM_EXPECT(std::abs(binMag - static_cast<AmReal32>(size) * 0.125f * 0.5f) < 0.5f);

            // Round trip with DC + Nyquist + mid-bin content must still reconstruct.
            AudioBuffer midBinOutput(size, 1);
            fft.Backward(midBinOutput[0].begin(), spectrum);
            for (AmSize i = 0; i < size; ++i)
                AM_EXPECT(std::abs(midBinOutput[0][i] - input[0][i]) < 1e-3f);
        }
    };

    AM_REGISTER_TEST(dsp_utilities, fft_real_layout_contract);
} // namespace SparkyStudios::Audio::Amplitude::Tests
