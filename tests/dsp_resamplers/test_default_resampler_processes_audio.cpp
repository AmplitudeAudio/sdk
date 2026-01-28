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
    AM_TEST_CASE(DSPTestCase, dsp_resamplers, default_resampler_processes_audio)
    {
    public:
        void Run() override
        {
            auto resampler = amshared(DefaultResampler);
            auto instance = resampler->CreateInstance();

            constexpr AmUInt16 channelCount = 2;
            constexpr AmUInt32 sampleRateIn = 44100;
            constexpr AmUInt32 sampleRateOut = 48000;
            constexpr AmUInt64 inputFrames = 1024;

            instance->Initialize(channelCount, sampleRateIn, sampleRateOut);

            // Create test audio buffers
            AudioBuffer inputBuffer(inputFrames, channelCount);
            const AmUInt64 expectedOutputFrames = instance->GetExpectedOutputFrames(inputFrames);
            AudioBuffer outputBuffer(expectedOutputFrames, channelCount);

            // Fill input buffer with test signal
            GenerateSineWave(inputBuffer, sampleRateIn);

            AmUInt64 processedInputFrames = inputFrames;
            AmUInt64 processedOutputFrames = expectedOutputFrames;

            // Process audio
            const bool result = instance->Process(inputBuffer, processedInputFrames, outputBuffer, processedOutputFrames);

            AM_EXPECT(result);
            AM_EXPECT(processedOutputFrames > 0);

            // Verify output buffer is not empty
            AM_EXPECT(EnsureHasNonZeroOutput(outputBuffer));
        }
    };

    AM_REGISTER_TEST(dsp_resamplers, default_resampler_processes_audio);
} // namespace SparkyStudios::Audio::Amplitude::Tests
