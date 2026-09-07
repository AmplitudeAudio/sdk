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

#include <DSP/Filters/EqualizerFilter.h>

#include "DSPTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(DSPTestCase, dsp_filters, equalizer_filter_high_bands_affect_high_frequencies)
    {
    public:
        void Run() override
        {
            constexpr AmUInt64 frameCount = 4096;
            constexpr AmUInt16 channelCount = 1;
            constexpr AmUInt32 sampleRate = 48000;

            auto filter = amshared(EqualizerFilter);
            filter->Initialize(1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 2.5f, 4.0f, 4.0f);

            // 15 kHz sits above fs/4 (12 kHz): bin 80 of 128. The buggy implementation
            // zeroed it entirely; a correct EQ applies the band-7 boost.
            {
                auto instance = filter->CreateInstance();
                AudioBuffer input(frameCount, channelCount);
                AudioBuffer output(frameCount, channelCount);
                GenerateSineWaveAtFrequency(input, sampleRate, 15000.0f, 0.5f);

                instance->Process(input, output, frameCount, sampleRate);

                const AmReal32 gain = CalculateRMS(output) / CalculateRMS(input);
                AM_EXPECT(gain > 1.5f);
            }

            // 500 Hz lands in band 1 (unity): must pass through at ~unity gain.
            {
                auto instance = filter->CreateInstance();
                AudioBuffer input(frameCount, channelCount);
                AudioBuffer output(frameCount, channelCount);
                GenerateSineWaveAtFrequency(input, sampleRate, 500.0f, 0.5f);

                instance->Process(input, output, frameCount, sampleRate);

                const AmReal32 gain = CalculateRMS(output) / CalculateRMS(input);
                AM_EXPECT(gain > 0.7f && gain < 1.4f);
            }
        }
    };

    AM_REGISTER_TEST(dsp_filters, equalizer_filter_high_bands_affect_high_frequencies);
} // namespace SparkyStudios::Audio::Amplitude::Tests
