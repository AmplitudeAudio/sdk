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
#include "TestRegistry.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(DSPTestCase, dsp_utilities, audio_converter_sample_rate_conversion)
    {
    public:
        void Run() override
        {
            AudioConverter converter;

            // Configure for sample rate conversion (44.1kHz to 48kHz)
            AudioConverter::Settings settings;
            settings.m_sourceSampleRate = 44100;
            settings.m_targetSampleRate = 48000;
            settings.m_sourceChannelCount = 2;
            settings.m_targetChannelCount = 2;

            AM_EXPECT(converter.Configure(settings));

            // Create test buffers
            constexpr AmUInt64 inputFrames = 1024;
            AudioBuffer input(inputFrames, 2);

            // Generate test signal
            for (AmUInt16 c = 0; c < 2; ++c)
            for (AmUInt64 i = 0; i < inputFrames; ++i)
            input[c][i] = std::sin(2.0f * AM_PI32 * 440.0f * static_cast<AmReal32>(i) / 44100.0f);

            // Calculate expected output frame count
            const AmUInt64 expectedOutputFrames = converter.GetExpectedOutputFrameCount(inputFrames);
            AM_EXPECT(expectedOutputFrames > inputFrames); // Should be upsampled

            AudioBuffer output(expectedOutputFrames, 2);
            AmUInt64 actualInputFrames = inputFrames;
            AmUInt64 actualOutputFrames = expectedOutputFrames;

            // Process
            converter.Process(input, actualInputFrames, output, actualOutputFrames);

            // Verify output
            AM_EXPECT(actualOutputFrames > 0);
            AM_EXPECT(EnsureHasNonZeroOutput(output));
        }
    };

    AM_REGISTER_TEST(dsp_utilities, audio_converter_sample_rate_conversion);
} // namespace SparkyStudios::Audio::Amplitude::Tests
