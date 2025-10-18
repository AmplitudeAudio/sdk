// Copyright (c) 2024-present Sparky Studios. All rights reserved.
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

namespace SparkyStudios::Audio::Amplitude::Tests
{
    void DSPTestCase::Run()
    {
        FFT fft;
        constexpr AmSize size = 512;
        fft.Initialize(size);

        // Create test signal
        AudioBuffer input(size, 1);
        AudioBuffer output(size, 1);

        // Generate sine wave
        GenerateSineWave(input, size);

        // Forward FFT
        SplitComplex splitComplex;
        fft.Forward(input[0].begin(), splitComplex);

        // Verify complex output
        AM_EXPECT(splitComplex.GetSize() > 0);
        AM_EXPECT(splitComplex.re() != nullptr);
        AM_EXPECT(splitComplex.im() != nullptr);

        // Backward FFT
        fft.Backward(output[0].begin(), splitComplex);

        // Verify reconstruction (allowing for some numerical error)
        constexpr AmReal32 tolerance = 0.01f;
        for (AmSize i = 0; i < size; ++i)
            AM_EXPECT(std::abs(output[0][i] - input[0][i]) < tolerance);
    }
} // namespace SparkyStudios::Audio::Amplitude::Tests
