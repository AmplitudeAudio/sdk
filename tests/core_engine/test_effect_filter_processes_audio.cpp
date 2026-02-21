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

#include "EngineTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(EngineTestCase, core_engine, effect_filter_processes_audio)
    {
    public:
        void Run() override
        {
            constexpr AmUInt64 frameCount = 4096;
            constexpr AmUInt16 channelCount = 1;
            constexpr AmUInt32 sampleRate = 48000;

            // Load the LPF effect (BiquadResonant low-pass at 2400 Hz)
            auto* effect = amEngine->GetEffectHandle("lpf");
            AM_EXPECT_NOT(effect == nullptr);

            auto instance = effect->CreateInstance();
            AM_EXPECT_NOT(instance == nullptr);

            auto filter = instance->GetFilter();
            AM_EXPECT_NOT(filter == nullptr);

            // Generate input with low (200 Hz) and high (8000 Hz) content
            AudioBuffer input(frameCount, channelCount);
            AudioBuffer output(frameCount, channelCount);

            for (AmUInt64 i = 0; i < frameCount; ++i)
            {
                const AmReal32 t = static_cast<AmReal32>(i) / static_cast<AmReal32>(sampleRate);
                input[0][i] = 0.4f * std::sin(2.0f * AM_PI32 * 200.0f * t)
                            + 0.4f * std::sin(2.0f * AM_PI32 * 8000.0f * t);
            }

            // Calculate input RMS
            AmReal64 inputSum = 0.0;
            for (AmUInt64 i = 0; i < frameCount; ++i)
                inputSum += input[0][i] * input[0][i];
            AmReal32 inputRMS = static_cast<AmReal32>(std::sqrt(inputSum / frameCount));

            // Process through the LPF
            filter->Process(input, output, frameCount, sampleRate);

            // Calculate output RMS
            AmReal64 outputSum = 0.0;
            for (AmUInt64 i = 0; i < frameCount; ++i)
                outputSum += output[0][i] * output[0][i];
            AmReal32 outputRMS = static_cast<AmReal32>(std::sqrt(outputSum / frameCount));

            // Output should not be silent (200 Hz passes through the 2400 Hz LPF)
            AM_EXPECT(outputRMS > 0.01f);

            // Output RMS should be less than input (8000 Hz attenuated by LPF)
            AM_EXPECT(outputRMS < inputRMS);
        }
    };

    AM_REGISTER_TEST(core_engine, effect_filter_processes_audio);
} // namespace SparkyStudios::Audio::Amplitude::Tests
