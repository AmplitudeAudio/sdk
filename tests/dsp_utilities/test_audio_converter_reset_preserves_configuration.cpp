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

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(DSPTestCase, dsp_utilities, audio_converter_reset_preserves_configuration)
    {
    public:
        void Run() override
        {
            AudioConverter converter;

            AudioConverter::Settings settings;
            settings.m_sourceSampleRate = 48000;
            settings.m_targetSampleRate = 48000;
            settings.m_sourceChannelCount = 1;
            settings.m_targetChannelCount = 2;

            AM_EXPECT(converter.Configure(settings));
            converter.Reset();

            constexpr AmUInt64 frameCount = 16;
            AudioBuffer input(frameCount, 1);
            AudioBuffer output(frameCount, 2);

            for (AmUInt64 i = 0; i < frameCount; ++i)
                input[0][i] = 1.0f;

            AmUInt64 inputFrames = frameCount;
            AmUInt64 outputFrames = frameCount;
            converter.Process(input, inputFrames, output, outputFrames);

            AM_EXPECT(EnsureHasNonZeroOutput(output));
            AM_EXPECT(output.GetChannelCount() == 2);
            AM_EXPECT(output[1][0] > 0.0f);
        }
    };

    AM_REGISTER_TEST(dsp_utilities, audio_converter_reset_preserves_configuration);
} // namespace SparkyStudios::Audio::Amplitude::Tests
