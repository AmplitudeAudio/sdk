// Copyright (c) 2021-present Sparky Studios. All rights reserved.
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

#include <DSP/Resamplers/DefaultResampler.h>

#include "DSPTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(DSPTestCase, dsp_resamplers, default_resampler_handles_hostile_sample_rates)
    {
    public:
        void Run() override
        {
            // Legacy DAT rate, legacy Mac rate, a rare broadcast rate, and nearby primes.
            constexpr AmUInt32 hostileRates[] = { 44056, 22254, 32075, 44059, 47999, 96001 };
            constexpr AmUInt32 targetSampleRate = 48000;
            constexpr AmUInt16 channelCount = 1;
            constexpr AmUInt64 inputFrames = 1024;

            auto resampler = amshared(DefaultResampler);

            for (const AmUInt32 sourceSampleRate : hostileRates)
            {
                auto instance = std::static_pointer_cast<DefaultResamplerInstance>(resampler->CreateInstance());

                instance->Initialize(channelCount, sourceSampleRate, targetSampleRate);

                AM_EXPECT(instance->GetUpRate() <= kMaxPolyphaseRate);
                AM_EXPECT(instance->GetDownRate() <= kMaxPolyphaseRate);

                AudioBuffer inputBuffer(inputFrames, channelCount);
                GenerateSineWave(inputBuffer, sourceSampleRate);

                const AmUInt64 expectedOutputFrames = instance->GetExpectedOutputFrames(inputFrames);
                AudioBuffer outputBuffer(expectedOutputFrames, channelCount);

                AmUInt64 processedInputFrames = inputFrames;
                AmUInt64 processedOutputFrames = expectedOutputFrames;

                AM_EXPECT(instance->Process(inputBuffer, processedInputFrames, outputBuffer, processedOutputFrames));
                AM_EXPECT(processedOutputFrames > 0);
                AM_EXPECT(processedOutputFrames <= expectedOutputFrames);
            }
        }
    };

    AM_REGISTER_TEST(dsp_resamplers, default_resampler_handles_hostile_sample_rates);
} // namespace SparkyStudios::Audio::Amplitude::Tests
