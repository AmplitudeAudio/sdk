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

#include <DSP/Filters/DelayFilter.h>

#include "DSPTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(DSPTestCase, dsp_filters, delay_filter_produces_echo)
    {
    public:
        void Run() override
        {
            constexpr AmUInt64 frameCount = 4096;
            constexpr AmUInt16 channelCount = 1;
            constexpr AmUInt32 sampleRate = 48000;

            // Configure delay: 0.01s delay with 0.5 decay (echo mode: delayStart=0)
            auto filter = amshared(DelayFilter);
            filter->Initialize(0.01f, 0.5f, 0.0f);
            auto instance = filter->CreateInstance();
            AM_EXPECT_NOT(instance == nullptr);

            // Full wet signal
            instance->SetParameter(DelayFilter::ATTRIBUTE_WET, 1.0f);

            // Generate impulse input — echo should produce delayed copies
            AudioBuffer input(frameCount, channelCount);
            GenerateImpulse(input, 1.0f, 0);

            AudioBuffer output(frameCount, channelCount);
            instance->Process(input, output, frameCount, sampleRate);

            ExpectNotSilent(output, 0.001f);

            // The delay adds energy from the echo — RMS of output should exceed
            // what a single impulse would produce (due to the delayed + decayed copies)
            AmReal32 outputPeak = CalculatePeak(output);
            AM_EXPECT(outputPeak > 0.1f);

            // Verify that there's signal energy beyond the initial impulse region
            // (the echo appears later in the buffer)
            AmUInt64 delaySamples = static_cast<AmUInt64>(0.01f * static_cast<AmReal32>(sampleRate));
            bool hasEcho = false;
            for (AmUInt64 i = delaySamples / 2; i < frameCount; ++i)
            {
                if (std::abs(output[0][i]) > 0.01f)
                {
                    hasEcho = true;
                    break;
                }
            }
            AM_EXPECT(hasEcho);
        }
    };

    AM_REGISTER_TEST(dsp_filters, delay_filter_produces_echo);
} // namespace SparkyStudios::Audio::Amplitude::Tests
