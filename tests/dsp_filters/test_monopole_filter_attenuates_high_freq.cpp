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

#include <DSP/Filters/MonoPoleFilter.h>

#include "DSPTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(DSPTestCase, dsp_filters, monopole_filter_attenuates_high_freq)
    {
    public:
        void Run() override
        {
            constexpr AmUInt64 frameCount = 4096;
            constexpr AmUInt16 channelCount = 1;
            constexpr AmUInt32 sampleRate = 48000;

            // MonoPole with coefficient 0.5 acts as simple low-pass filter
            auto filter = amshared(MonoPoleFilter);
            filter->Initialize(0.5f);

            // Process low-frequency signal (100 Hz) — should pass mostly unchanged
            {
                auto instance = filter->CreateInstance();
                AudioBuffer lowInput(frameCount, channelCount);
                AudioBuffer lowOutput(frameCount, channelCount);
                GenerateSineWaveAtFrequency(lowInput, sampleRate, 100.0f, 0.5f);

                instance->Process(lowInput, lowOutput, frameCount, sampleRate);

                AmReal32 lowInputRMS = CalculateRMS(lowInput);
                AmReal32 lowOutputRMS = CalculateRMS(lowOutput);

                // Low frequency should pass with reasonable energy
                AM_EXPECT(lowOutputRMS > lowInputRMS * 0.3f);

                // Process high-frequency signal (10000 Hz) — should be attenuated
                auto instance2 = filter->CreateInstance();
                AudioBuffer highInput(frameCount, channelCount);
                AudioBuffer highOutput(frameCount, channelCount);
                GenerateSineWaveAtFrequency(highInput, sampleRate, 10000.0f, 0.5f);

                instance2->Process(highInput, highOutput, frameCount, sampleRate);

                AmReal32 highInputRMS = CalculateRMS(highInput);
                AmReal32 highOutputRMS = CalculateRMS(highOutput);

                // High frequency should be more attenuated than low frequency
                AmReal32 lowPassRatio = lowOutputRMS / lowInputRMS;
                AmReal32 highPassRatio = highOutputRMS / highInputRMS;
                AM_EXPECT(lowPassRatio > highPassRatio);
            }
        }
    };

    AM_REGISTER_TEST(dsp_filters, monopole_filter_attenuates_high_freq);
} // namespace SparkyStudios::Audio::Amplitude::Tests
