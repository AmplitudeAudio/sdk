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
        std::vector<AmAudioSample> input(size);
        std::vector<AmAudioSample> output(size);

        // Generate sine wave
        for (AmSize i = 0; i < size; ++i)
            input[i] = std::sin(2.0f * AM_PI32 * 10.0f * static_cast<AmReal32>(i) / static_cast<AmReal32>(size));

        // Forward FFT
        SplitComplex splitComplex;
        fft.Forward(input.data(), splitComplex);

        // Verify complex output
        AM_EXPECT(splitComplex.GetSize() > 0);
        AM_EXPECT(splitComplex.re() != nullptr);
        AM_EXPECT(splitComplex.im() != nullptr);

        // Backward FFT
        fft.Backward(output.data(), splitComplex);

        // Verify reconstruction (allowing for some numerical error)
        constexpr AmReal32 tolerance = 0.01f;
        for (AmSize i = 0; i < size; ++i)
            AM_EXPECT(std::abs(output[i] - input[i]) < tolerance);
    }
} // namespace SparkyStudios::Audio::Amplitude::Tests
