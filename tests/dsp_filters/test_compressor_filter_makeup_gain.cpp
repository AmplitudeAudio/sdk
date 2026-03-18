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
    AM_TEST_CASE(DSPTestCase, dsp_filters, compressor_filter_makeup_gain)
    {
    public:
        void Run() override
        {
            constexpr AmUInt64 frameCount = 4096;
            constexpr AmUInt16 channelCount = 1;
            constexpr AmUInt32 sampleRate = 48000;

            AudioBuffer input(frameCount, channelCount);
            // Quiet signal well below threshold
            GenerateSineWaveAtFrequency(input, sampleRate, 1000.0f, 0.05f);

            // No compression (below threshold), but 12dB makeup gain (~4x)
            auto filter = amshared(CompressorFilter);
            filter->Initialize(-6.0f, 4.0f, 5.0f, 100.0f, 0.0f, 12.0f);
            auto instance = filter->CreateInstance();

            AudioBuffer output(frameCount, channelCount);
            instance->Process(input, output, frameCount, sampleRate);

            AmReal32 inputRMS = CalculateRMS(input);
            AmReal32 outputRMS = CalculateRMS(output);

            // Makeup gain should boost the output above the input
            AM_EXPECT(outputRMS > inputRMS * 2.0f);
        }
    };

    AM_REGISTER_TEST(dsp_filters, compressor_filter_makeup_gain);
} // namespace SparkyStudios::Audio::Amplitude::Tests
