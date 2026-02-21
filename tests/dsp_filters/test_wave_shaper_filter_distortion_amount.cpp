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

#include <DSP/Filters/WaveShaperFilter.h>

#include "DSPTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(DSPTestCase, dsp_filters, wave_shaper_filter_distortion_amount)
    {
    public:
        void Run() override
        {
            constexpr AmUInt64 frameCount = 2048;
            constexpr AmUInt16 channelCount = 1;
            constexpr AmUInt32 sampleRate = 48000;

            AudioBuffer input(frameCount, channelCount);
            GenerateSineWaveAtFrequency(input, sampleRate, 440.0f, 0.8f);

            // Test: Dry signal (wet=0) should equal input
            {
                auto filter = amshared(WaveShaperFilter);
                filter->Initialize(0.5f);
                auto instance = filter->CreateInstance();
                instance->SetParameter(WaveShaperFilter::ATTRIBUTE_WET, 0.0f);

                AudioBuffer output(frameCount, channelCount);
                instance->Process(input, output, frameCount, sampleRate);

                AM_EXPECT(EnsureBufferEqual(input, output));
            }

            // Test: Increasing distortion amount should increase signal difference
            {
                auto filterLow = amshared(WaveShaperFilter);
                filterLow->Initialize(0.2f);
                auto instanceLow = filterLow->CreateInstance();

                AudioBuffer outputLow(frameCount, channelCount);
                instanceLow->Process(input, outputLow, frameCount, sampleRate);

                auto filterHigh = amshared(WaveShaperFilter);
                filterHigh->Initialize(0.9f);
                auto instanceHigh = filterHigh->CreateInstance();

                AudioBuffer outputHigh(frameCount, channelCount);
                instanceHigh->Process(input, outputHigh, frameCount, sampleRate);

                // Both should differ from input
                AM_EXPECT_NOT(EnsureBufferEqual(input, outputLow));
                AM_EXPECT_NOT(EnsureBufferEqual(input, outputHigh));

                // Higher amount should produce more deviation from input
                AmReal32 lowDiff = 0.0f;
                AmReal32 highDiff = 0.0f;
                for (AmUInt64 i = 0; i < frameCount; ++i)
                {
                    lowDiff += std::abs(outputLow[0][i] - input[0][i]);
                    highDiff += std::abs(outputHigh[0][i] - input[0][i]);
                }

                AM_EXPECT(highDiff > lowDiff);
            }
        }
    };

    AM_REGISTER_TEST(dsp_filters, wave_shaper_filter_distortion_amount);
} // namespace SparkyStudios::Audio::Amplitude::Tests
