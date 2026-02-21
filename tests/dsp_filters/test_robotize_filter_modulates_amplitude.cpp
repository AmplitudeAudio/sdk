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

#include <DSP/Filters/RobotizeFilter.h>

#include "DSPTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(DSPTestCase, dsp_filters, robotize_filter_modulates_amplitude)
    {
    public:
        void Run() override
        {
            constexpr AmUInt64 frameCount = 4096;
            constexpr AmUInt16 channelCount = 1;
            constexpr AmUInt32 sampleRate = 48000;

            AudioBuffer input(frameCount, channelCount);
            GenerateSineWaveAtFrequency(input, sampleRate, 440.0f, 0.5f);

            AmReal32 inputRMS = CalculateRMS(input);

            // Test with sine waveform modulation — should reduce overall RMS
            // (multiplying by a sine carrier with values [0,1] reduces average energy)
            {
                auto filter = amshared(RobotizeFilter);
                filter->Initialize(30.0f, RobotizeFilter::WAVE_SIN);
                auto instance = filter->CreateInstance();

                AudioBuffer output(frameCount, channelCount);
                instance->Process(input, output, frameCount, sampleRate);

                ExpectNotSilent(output, 0.001f);

                // Amplitude modulation generally reduces RMS
                AmReal32 outputRMS = CalculateRMS(output);
                AM_EXPECT(outputRMS < inputRMS);
            }

            // Test with square waveform — should create silence in parts of the signal
            {
                auto filter = amshared(RobotizeFilter);
                filter->Initialize(30.0f, RobotizeFilter::WAVE_SQUARE);
                auto instance = filter->CreateInstance();

                AudioBuffer output(frameCount, channelCount);
                instance->Process(input, output, frameCount, sampleRate);

                // Square wave modulation alternates between full signal and silence
                // Check that some samples are near-zero (during the off phase)
                AmUInt32 silentSamples = 0;
                for (AmUInt64 i = 0; i < frameCount; ++i)
                {
                    if (std::abs(output[0][i]) < 0.01f)
                        ++silentSamples;
                }

                // Should have some silent regions from the square wave gating
                AM_EXPECT(silentSamples > frameCount / 8);
            }
        }
    };

    AM_REGISTER_TEST(dsp_filters, robotize_filter_modulates_amplitude);
} // namespace SparkyStudios::Audio::Amplitude::Tests
