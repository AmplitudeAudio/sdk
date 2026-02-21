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

#include <DSP/Filters/LofiFilter.h>

#include "DSPTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(DSPTestCase, dsp_filters, lofi_filter_degrades_signal)
    {
    public:
        void Run() override
        {
            constexpr AmUInt64 frameCount = 4096;
            constexpr AmUInt16 channelCount = 1;
            constexpr AmUInt32 sampleRate = 48000;

            AudioBuffer input(frameCount, channelCount);
            GenerateSineWaveAtFrequency(input, sampleRate, 440.0f, 0.5f);

            // Test 1: Aggressive lo-fi (low sample rate, low bit depth) should differ from input
            {
                auto filter = amshared(LofiFilter);
                filter->Initialize(4000.0f, 4.0f);
                auto instance = filter->CreateInstance();

                AudioBuffer output(frameCount, channelCount);
                instance->Process(input, output, frameCount, sampleRate);

                ExpectNotSilent(output, 0.01f);
                AM_EXPECT_NOT(EnsureBufferEqual(input, output));
            }

            // Test 2: More aggressive degradation should produce more difference from input
            {
                auto mildFilter = amshared(LofiFilter);
                mildFilter->Initialize(24000.0f, 16.0f); // Mild: high sample rate, high bit depth
                auto mildInstance = mildFilter->CreateInstance();

                AudioBuffer mildOutput(frameCount, channelCount);
                mildInstance->Process(input, mildOutput, frameCount, sampleRate);

                auto aggressiveFilter = amshared(LofiFilter);
                aggressiveFilter->Initialize(4000.0f, 4.0f); // Aggressive: low sample rate, low bit depth
                auto aggressiveInstance = aggressiveFilter->CreateInstance();

                AudioBuffer aggressiveOutput(frameCount, channelCount);
                aggressiveInstance->Process(input, aggressiveOutput, frameCount, sampleRate);

                // Calculate difference from input for each
                AmReal32 mildDiff = 0.0f;
                AmReal32 aggressiveDiff = 0.0f;
                for (AmUInt64 i = 0; i < frameCount; ++i)
                {
                    mildDiff += std::abs(mildOutput[0][i] - input[0][i]);
                    aggressiveDiff += std::abs(aggressiveOutput[0][i] - input[0][i]);
                }

                // Aggressive lo-fi should differ more from original
                AM_EXPECT(aggressiveDiff > mildDiff);
            }
        }
    };

    AM_REGISTER_TEST(dsp_filters, lofi_filter_degrades_signal);
} // namespace SparkyStudios::Audio::Amplitude::Tests
