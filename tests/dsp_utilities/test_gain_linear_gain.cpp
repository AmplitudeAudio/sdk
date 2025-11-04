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
        constexpr AmSize frameCount = 100;
        AudioBuffer input(frameCount, 1);
        AudioBuffer output(frameCount, 1);

        // Fill input with constant value
        for (AmSize i = 0; i < frameCount; ++i)
            input[0][i] = 1.0f;

        // Test linear ramp from 0.0 to 1.0
        constexpr AmReal32 startGain = 0.0f;
        constexpr AmReal32 endGain = 1.0f;

        Gain::ApplyReplaceLinearGain(startGain, endGain, input[0], 0, output[0], 0, frameCount);

        // Verify linear interpolation
        AM_EXPECT(std::abs(output[0][0] - 0.0f) < 0.01f); // Start near 0
        AM_EXPECT(std::abs(output[0][frameCount / 2] - 0.5f) < 0.01f); // Middle near 0.5
        AM_EXPECT(std::abs(output[0][frameCount - 1] - 0.99f) < 0.01f); // End near 1.0
    }
} // namespace SparkyStudios::Audio::Amplitude::Tests
