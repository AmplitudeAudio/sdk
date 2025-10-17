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

#include <DSP/Gain.h>

#include "DSPTestCase.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    void DSPTestCase::Run()
    {
        constexpr AmSize frameCount = 256;
        AudioBuffer input(frameCount, 1);
        AudioBuffer output(frameCount, 1);

        // Fill input with constant value
        for (AmSize i = 0; i < frameCount; ++i)
            input[0][i] = 1.0f;

        // Test replace with gain of 0.5
        constexpr AmReal32 gain = 0.5f;
        Gain::ApplyReplaceConstantGain(gain, input[0], 0, output[0], 0, frameCount);

        // Verify output
        for (AmSize i = 0; i < frameCount; ++i)
            AM_EXPECT(std::abs(output[0][i] - 0.5f) < kEpsilon);

        // Test accumulate
        output[0].clear();
        for (AmSize i = 0; i < frameCount; ++i)
            output[0][i] = 0.25f;

        Gain::ApplyAccumulateConstantGain(gain, input[0], 0, output[0], 0, frameCount);

        // Verify accumulated output (0.25 + 0.5 = 0.75)
        for (AmSize i = 0; i < frameCount; ++i)
            AM_EXPECT(std::abs(output[0][i] - 0.75f) < kEpsilon);
    }
} // namespace SparkyStudios::Audio::Amplitude::Tests
