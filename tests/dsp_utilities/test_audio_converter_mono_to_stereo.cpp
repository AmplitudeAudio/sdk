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
        AudioConverter converter;

        // Configure for mono to stereo conversion (same sample rate)
        AudioConverter::Settings settings;
        settings.m_sourceSampleRate = 48000;
        settings.m_targetSampleRate = 48000;
        settings.m_sourceChannelCount = 1;
        settings.m_targetChannelCount = 2;

        AM_EXPECT(converter.Configure(settings));

        // Create test buffers
        constexpr AmUInt64 frameCount = 512;
        AudioBuffer input(frameCount, 1);
        AudioBuffer output(frameCount, 2);

        // Generate test signal (mono)
        for (AmUInt64 i = 0; i < frameCount; ++i)
            input[0][i] = static_cast<AmReal32>(i) / static_cast<AmReal32>(frameCount);

        AmUInt64 inputFrames = frameCount;
        AmUInt64 outputFrames = frameCount;

        // Process
        converter.Process(input, inputFrames, output, outputFrames);

        // Verify stereo output
        AM_EXPECT(output.GetChannelCount() == 2);
        AM_EXPECT(EnsureHasNonZeroOutput(output));

        // Left and right channels should have similar values (scaled by 1/sqrt(2))
        constexpr AmReal32 expectedScale = 0.7071f; // 1/sqrt(2)
        constexpr AmReal32 tolerance = 0.01f;

        for (AmUInt64 i = 0; i < 10; ++i) // Check first 10 samples
        {
            const AmReal32 expectedValue = input[0][i] * expectedScale;
            AM_EXPECT(std::abs(output[0][i] - expectedValue) < tolerance);
            AM_EXPECT(std::abs(output[1][i] - expectedValue) < tolerance);
        }
    }
} // namespace SparkyStudios::Audio::Amplitude::Tests
