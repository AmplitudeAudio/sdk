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
    AM_TEST_CASE(DSPTestCase, dsp_filters, compressor_filter_below_threshold_passthrough)
    {
    public:
        void Run() override
        {
            constexpr AmUInt64 frameCount = 4096;
            constexpr AmUInt16 channelCount = 1;
            constexpr AmUInt32 sampleRate = 48000;

            auto filter = amshared(CompressorFilter);
            // Threshold at -6dB (~0.5 linear), signal at 0.1 amplitude (~-20dB) — well below threshold
            filter->Initialize(-6.0f, 4.0f, 5.0f, 100.0f, 0.0f);
            auto instance = filter->CreateInstance();

            AudioBuffer input(frameCount, channelCount);
            AudioBuffer output(frameCount, channelCount);

            GenerateSineWaveAtFrequency(input, sampleRate, 1000.0f, 0.1f);

            instance->Process(input, output, frameCount, sampleRate);

            // Signal below threshold should pass through with unity gain (no compression)
            AmReal32 inputRMS = CalculateRMS(input);
            AmReal32 outputRMS = CalculateRMS(output);

            // Allow small tolerance for envelope follower settling
            AM_EXPECT(std::abs(outputRMS - inputRMS) / inputRMS < 0.05f);
        }
    };

    AM_REGISTER_TEST(dsp_filters, compressor_filter_below_threshold_passthrough);
} // namespace SparkyStudios::Audio::Amplitude::Tests
