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

#include <DSP/Filters/BiquadResonantFilter.h>
#include <DSP/NearFieldProcessor.h>

#include "DSPTestCase.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    void DSPTestCase::Run()
    {
        auto biquadFilter = Engine::RegisterExtension<BiquadResonantFilter>();

        constexpr AmUInt32 sampleRate = 48000;
        constexpr AmUInt32 framesCount = 512;

        NearFieldProcessor processor(sampleRate, framesCount);

        // Create input and output buffers
        AudioBuffer input(framesCount, 1);
        AudioBuffer output(framesCount, 1);

        // Generate test signal
        for (AmSize i = 0; i < framesCount; ++i)
            input[0][i] = std::sin(2.0f * AM_PI32 * 440.0f * static_cast<AmReal32>(i) / static_cast<AmReal32>(sampleRate));

        // Process with HRTF disabled
        processor.Process(input[0], output[0], false);

        // Verify output is non-zero and different from input
        AM_EXPECT(EnsureHasNonZeroOutput(output));
        AM_EXPECT_NOT(EnsureBufferEqual(input, output));

        Engine::UnregisterExtension(biquadFilter);
    }
} // namespace SparkyStudios::Audio::Amplitude::Tests
