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
        // Create an impulse response
        constexpr AmSize irLen = 512;
        std::vector<AmAudioSample> ir(irLen);

        // Simple decaying impulse
        for (AmSize i = 0; i < irLen; ++i)
            ir[i] = std::exp(-static_cast<AmReal32>(i) / 100.0f);

        Convolver convolver;
        constexpr AmSize blockSize = 256;
        AM_EXPECT(convolver.Init(blockSize, ir.data(), irLen));

        // Create input signal
        constexpr AmSize inputLen = 1024;
        std::vector<AmAudioSample> input(inputLen);
        std::vector<AmAudioSample> output(inputLen);

        // Generate test signal (impulse)
        input[0] = 1.0f;
        for (AmSize i = 1; i < inputLen; ++i)
            input[i] = 0.0f;

        // Process
        convolver.Process(input.data(), output.data(), inputLen);

        // Verify output is non-zero and different from input
        bool hasOutput = false;
        bool isDifferent = false;

        for (AmSize i = 0; i < inputLen; ++i)
        {
            if (std::abs(output[i]) > kEpsilon)
                hasOutput = true;

            if (std::abs(output[i] - input[i]) > kEpsilon)
                isDifferent = true;
        }

        AM_EXPECT(hasOutput);
        AM_EXPECT(isDifferent);
    }
} // namespace SparkyStudios::Audio::Amplitude::Tests
