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

#include <cmath>
#include <vector>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <DSP/Resamplers/DefaultResampler.h>

#include "DSPTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(DSPTestCase, dsp_resamplers, default_resampler_carries_state_across_small_blocks)
    {
    public:
        void Run() override
        {
            // Upsampling 8000 Hz to 48000 Hz with a small device buffer asks for fewer input frames than the
            // filter history (_coefficientsPerPhase - 1 = 13), which takes the remainingSamples > 0 branch of
            // Process(). That branch shifts the history relative to the end of the state channel, so it is only
            // correct while the channel capacity equals the history length.
            constexpr AmUInt16 channelCount = 1;
            constexpr AmUInt32 sampleRateIn = 8000;
            constexpr AmUInt32 sampleRateOut = 48000;
            constexpr AmUInt64 blockFrames = 8;
            constexpr AmUInt32 blockCount = 64;
            constexpr AmReal32 frequency = 200.0f;

            auto resampler = amshared(DefaultResampler);
            auto instance = resampler->CreateInstance();

            instance->Initialize(channelCount, sampleRateIn, sampleRateOut);

            std::vector<AmReal32> resampled;
            AmUInt64 phase = 0;

            for (AmUInt32 block = 0; block < blockCount; ++block)
            {
                AudioBuffer inputBuffer(blockFrames, channelCount);
                for (AmUInt64 i = 0; i < blockFrames; ++i, ++phase)
                    inputBuffer[0][i] =
                        std::sin(2.0f * AM_PI32 * frequency * static_cast<AmReal32>(phase) / static_cast<AmReal32>(sampleRateIn));

                const AmUInt64 expectedOutputFrames = instance->GetExpectedOutputFrames(blockFrames);
                AudioBuffer outputBuffer(expectedOutputFrames, channelCount);

                AmUInt64 processedInputFrames = blockFrames;
                AmUInt64 processedOutputFrames = expectedOutputFrames;

                AM_EXPECT(instance->Process(inputBuffer, processedInputFrames, outputBuffer, processedOutputFrames));

                for (AmUInt64 i = 0; i < processedOutputFrames; ++i)
                    resampled.push_back(outputBuffer[0][i]);
            }

            // A 200 Hz sine sampled at 48 kHz moves at most 0.027 per sample. A lost state carry produces jumps
            // close to the full amplitude, so a 0.1 threshold separates the two cases with a wide margin.
            AM_EXPECT(resampled.size() > 256);

            AmReal32 largestJump = 0.0f;
            for (AmSize i = 128; i < resampled.size(); ++i)
            {
                const AmReal32 jump = std::abs(resampled[i] - resampled[i - 1]);
                if (jump > largestJump)
                    largestJump = jump;
            }

            AM_EXPECT(largestJump < 0.1f);
        }
    };

    AM_REGISTER_TEST(dsp_resamplers, default_resampler_carries_state_across_small_blocks);
} // namespace SparkyStudios::Audio::Amplitude::Tests
