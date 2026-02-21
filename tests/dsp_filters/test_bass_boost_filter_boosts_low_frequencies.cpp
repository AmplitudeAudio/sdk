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

#include <DSP/Filters/BassBoostFilter.h>

#include "DSPTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(DSPTestCase, dsp_filters, bass_boost_filter_boosts_low_frequencies)
    {
    public:
        void Run() override
        {
            constexpr AmUInt64 frameCount = 4096;
            constexpr AmUInt16 channelCount = 1;
            constexpr AmUInt32 sampleRate = 48000;

            auto filter = amshared(BassBoostFilter);
            filter->Initialize(2.0f);
            auto instance = filter->CreateInstance();
            AM_EXPECT_NOT(instance == nullptr);

            // Process a low-frequency signal (100 Hz) — should be boosted
            AudioBuffer lowInput(frameCount, channelCount);
            AudioBuffer lowOutput(frameCount, channelCount);
            GenerateSineWaveAtFrequency(lowInput, sampleRate, 100.0f, 0.5f);

            instance->Process(lowInput, lowOutput, frameCount, sampleRate);

            AmReal32 lowInputRMS = CalculateRMS(lowInput);
            AmReal32 lowOutputRMS = CalculateRMS(lowOutput);

            // Bass boost should increase low-frequency energy
            AM_EXPECT(lowOutputRMS > lowInputRMS);

            // Process a high-frequency signal (8000 Hz) — should not be boosted significantly
            auto instance2 = filter->CreateInstance();
            AudioBuffer highInput(frameCount, channelCount);
            AudioBuffer highOutput(frameCount, channelCount);
            GenerateSineWaveAtFrequency(highInput, sampleRate, 8000.0f, 0.5f);

            instance2->Process(highInput, highOutput, frameCount, sampleRate);

            AmReal32 highInputRMS = CalculateRMS(highInput);
            AmReal32 highOutputRMS = CalculateRMS(highOutput);

            // Bass boost ratio for low freq should be greater than for high freq
            AmReal32 lowBoostRatio = lowOutputRMS / lowInputRMS;
            AmReal32 highBoostRatio = highOutputRMS / highInputRMS;
            AM_EXPECT(lowBoostRatio > highBoostRatio);
        }
    };

    AM_REGISTER_TEST(dsp_filters, bass_boost_filter_boosts_low_frequencies);
} // namespace SparkyStudios::Audio::Amplitude::Tests
