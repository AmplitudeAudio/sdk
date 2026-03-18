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
    AM_TEST_CASE(DSPTestCase, dsp_filters, compressor_filter_wet_dry_mix)
    {
    public:
        void Run() override
        {
            constexpr AmUInt64 frameCount = 4096;
            constexpr AmUInt16 channelCount = 1;
            constexpr AmUInt32 sampleRate = 48000;

            AudioBuffer input(frameCount, channelCount);
            GenerateSineWaveAtFrequency(input, sampleRate, 1000.0f, 0.9f);

            // Fully wet (default)
            auto filterWet = amshared(CompressorFilter);
            filterWet->Initialize(-20.0f, 4.0f, 5.0f, 100.0f, 0.0f);
            auto instanceWet = filterWet->CreateInstance();

            AudioBuffer outputWet(frameCount, channelCount);
            instanceWet->Process(input, outputWet, frameCount, sampleRate);

            // Fully dry (wet = 0.0)
            auto filterDry = amshared(CompressorFilter);
            filterDry->Initialize(-20.0f, 4.0f, 5.0f, 100.0f, 0.0f);
            auto instanceDry = filterDry->CreateInstance();
            instanceDry->SetParameter(CompressorFilter::ATTRIBUTE_WET, 0.0f);

            AudioBuffer outputDry(frameCount, channelCount);
            instanceDry->Process(input, outputDry, frameCount, sampleRate);

            // Dry output should match input (no processing)
            AM_EXPECT(EnsureBufferEqual(input, outputDry));

            // Wet output should be different from input (compression applied)
            AM_EXPECT_NOT(EnsureBufferEqual(input, outputWet));

            // Wet output should have lower RMS than input
            AM_EXPECT(CalculateRMS(outputWet) < CalculateRMS(input));
        }
    };

    AM_REGISTER_TEST(dsp_filters, compressor_filter_wet_dry_mix);
} // namespace SparkyStudios::Audio::Amplitude::Tests
