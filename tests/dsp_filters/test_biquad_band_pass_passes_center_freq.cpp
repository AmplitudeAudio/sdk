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
    AM_TEST_CASE(DSPTestCase, dsp_filters, biquad_band_pass_passes_center_freq)
    {
    public:
        void Run() override
        {
            constexpr AmUInt64 frameCount = 4096;
            constexpr AmUInt16 channelCount = 1;
            constexpr AmUInt32 sampleRate = 48000;

            // Band-pass centered at 1000 Hz with Q=2
            auto filter = amshared(BiquadResonantFilter);
            filter->InitializeBandPass(1000.0f, 2.0f);

            // Test 1: Signal at center frequency should pass through
            {
                auto instance = filter->CreateInstance();
                AudioBuffer centerInput(frameCount, channelCount);
                AudioBuffer centerOutput(frameCount, channelCount);
                GenerateSineWaveAtFrequency(centerInput, sampleRate, 1000.0f, 0.5f);

                instance->Process(centerInput, centerOutput, frameCount, sampleRate);
                ExpectNotSilent(centerOutput, 0.01f);
                AmReal32 centerRMS = CalculateRMS(centerOutput);

                // Test 2: Signal far from center should be attenuated
                auto instance2 = filter->CreateInstance();
                AudioBuffer offInput(frameCount, channelCount);
                AudioBuffer offOutput(frameCount, channelCount);
                GenerateSineWaveAtFrequency(offInput, sampleRate, 100.0f, 0.5f);

                instance2->Process(offInput, offOutput, frameCount, sampleRate);
                AmReal32 offRMS = CalculateRMS(offOutput);

                // Center frequency should have more energy than off-center
                AM_EXPECT(centerRMS > offRMS);
            }

            // Test 3: Multi-tone through band-pass should differ from input
            // (off-center frequencies altered, center frequency preserved or boosted by Q)
            {
                auto instance = filter->CreateInstance();
                AudioBuffer input(frameCount, channelCount);
                AudioBuffer output(frameCount, channelCount);
                GenerateMultiTone(input, sampleRate, { 100.0f, 1000.0f, 8000.0f }, 0.3f);

                instance->Process(input, output, frameCount, sampleRate);

                AM_EXPECT_NOT(EnsureBufferEqual(input, output));
                ExpectNotSilent(output, 0.01f);
            }
        }
    };

    AM_REGISTER_TEST(dsp_filters, biquad_band_pass_passes_center_freq);
} // namespace SparkyStudios::Audio::Amplitude::Tests
