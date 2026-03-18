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

#include <DSP/Filters/CompressorFilter.h>

#include "DSPTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(DSPTestCase, dsp_filters, compressor_filter_stereo_processing)
    {
    public:
        void Run() override
        {
            constexpr AmUInt64 frameCount = 4096;
            constexpr AmUInt16 channelCount = 2;
            constexpr AmUInt32 sampleRate = 48000;

            auto filter = amshared(CompressorFilter);
            filter->Initialize(-12.0f, 4.0f, 5.0f, 100.0f, 0.0f);
            auto instance = filter->CreateInstance();

            AudioBuffer input(frameCount, channelCount);

            // Left: loud signal (above threshold)
            for (AmUInt64 i = 0; i < frameCount; ++i)
                input[0][i] = 0.9f * std::sin(2.0f * AM_PI32 * 440.0f * static_cast<AmReal32>(i) / static_cast<AmReal32>(sampleRate));

            // Right: quiet signal (below threshold)
            for (AmUInt64 i = 0; i < frameCount; ++i)
                input[1][i] = 0.05f * std::sin(2.0f * AM_PI32 * 880.0f * static_cast<AmReal32>(i) / static_cast<AmReal32>(sampleRate));

            AudioBuffer output(frameCount, channelCount);
            instance->Process(input, output, frameCount, sampleRate);

            // Both channels should have non-zero output
            ExpectNotSilent(output);

            // Left channel (loud) should be compressed
            AmReal32 leftInputPeak = 0.0f, leftOutputPeak = 0.0f;
            for (AmUInt64 i = frameCount / 2; i < frameCount; ++i)
            {
                leftInputPeak = std::max(leftInputPeak, std::abs(input[0][i]));
                leftOutputPeak = std::max(leftOutputPeak, std::abs(output[0][i]));
            }
            AM_EXPECT(leftOutputPeak < leftInputPeak);

            // Right channel (quiet, below threshold) should be mostly unchanged
            AmReal64 rightInputSum = 0.0, rightOutputSum = 0.0;
            for (AmUInt64 i = frameCount / 2; i < frameCount; ++i)
            {
                rightInputSum += static_cast<AmReal64>(input[1][i]) * static_cast<AmReal64>(input[1][i]);
                rightOutputSum += static_cast<AmReal64>(output[1][i]) * static_cast<AmReal64>(output[1][i]);
            }
            const AmReal64 halfFrames = static_cast<AmReal64>(frameCount) / 2.0;
            AmReal32 rightInputRMS = static_cast<AmReal32>(std::sqrt(rightInputSum / halfFrames));
            AmReal32 rightOutputRMS = static_cast<AmReal32>(std::sqrt(rightOutputSum / halfFrames));
            AM_EXPECT(std::abs(rightOutputRMS - rightInputRMS) / rightInputRMS < 0.1f);
        }
    };

    AM_REGISTER_TEST(dsp_filters, compressor_filter_stereo_processing);
} // namespace SparkyStudios::Audio::Amplitude::Tests
