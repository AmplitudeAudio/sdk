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

#include <DSP/Filters/BiquadResonantFilter.h>

#include "DSPTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(DSPTestCase, dsp_filters, biquad_low_pass_attenuates_high_freq)
    {
    public:
        void Run() override
        {
            constexpr AmUInt64 frameCount = 4096;
            constexpr AmUInt16 channelCount = 1;
            constexpr AmUInt32 sampleRate = 48000;

            auto filter = amshared(BiquadResonantFilter);
            filter->InitializeLowPass(1000.0f, 0.707107f);
            auto instance = filter->CreateInstance();
            AM_EXPECT_NOT(instance == nullptr);

            // Generate signal with 100 Hz (should pass) and 5000 Hz (should be attenuated)
            AudioBuffer input(frameCount, channelCount);
            GenerateMultiTone(input, sampleRate, { 100.0f, 5000.0f }, 0.4f);

            AudioBuffer output(frameCount, channelCount);
            instance->Process(input, output, frameCount, sampleRate);

            AmReal32 inputRMS = CalculateRMS(input);
            AmReal32 outputRMS = CalculateRMS(output);

            // High frequency attenuated: overall RMS should drop
            AM_EXPECT(outputRMS < inputRMS * 0.8f);

            // Low frequency still present: output should not be silent
            ExpectNotSilent(output, 0.01f);

            // No significant DC offset introduced
            ExpectNoDCOffset(output, 0.05f);
        }
    };

    AM_REGISTER_TEST(dsp_filters, biquad_low_pass_attenuates_high_freq);
} // namespace SparkyStudios::Audio::Amplitude::Tests
