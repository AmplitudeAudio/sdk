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
    AM_TEST_CASE(DSPTestCase, dsp_resamplers, default_resampler_state_is_continuous_across_blocks)
    {
    public:
        void Run() override
        {
            constexpr AmUInt16 channelCount = 1;
            constexpr AmUInt32 sampleRateIn = 44100;
            constexpr AmUInt32 sampleRateOut = 48000;
            constexpr AmUInt64 blockFrames = 512;
            constexpr AmUInt32 blockCount = 8;
            constexpr AmReal32 frequency = 1000.0f;

            auto resampler = amshared(DefaultResampler);
            auto instance = resampler->CreateInstance();

            instance->Initialize(channelCount, sampleRateIn, sampleRateOut);

            std::vector<AmReal32> resampled;
            AmUInt64 phase = 0;

            for (AmUInt32 block = 0; block < blockCount; ++block)
            {
                AudioBuffer inputBuffer(blockFrames, channelCount);
                for (AmUInt64 i = 0; i < blockFrames; ++i, ++phase)
                    inputBuffer[0][i] = std::sin(
                        2.0f * AM_PI32 * frequency * static_cast<AmReal32>(phase) / static_cast<AmReal32>(sampleRateIn));

                const AmUInt64 expectedOutputFrames = instance->GetExpectedOutputFrames(blockFrames);
                AudioBuffer outputBuffer(expectedOutputFrames, channelCount);

                AmUInt64 processedInputFrames = blockFrames;
                AmUInt64 processedOutputFrames = expectedOutputFrames;

                AM_EXPECT(instance->Process(inputBuffer, processedInputFrames, outputBuffer, processedOutputFrames));

                for (AmUInt64 i = 0; i < processedOutputFrames; ++i)
                    resampled.push_back(outputBuffer[0][i]);
            }

            // A 1 kHz sine resampled to 48 kHz moves at most ~0.14 per sample. Skip the filter warm-up region,
            // then assert no sample-to-sample jump exceeds a generous bound: a discontinuity at a block seam
            // caused by a lost state carry produces a jump close to the full amplitude.
            AM_EXPECT(resampled.size() > 64);

            AmReal32 largestJump = 0.0f;
            for (AmSize i = 32; i < resampled.size(); ++i)
            {
                const AmReal32 jump = std::abs(resampled[i] - resampled[i - 1]);
                if (jump > largestJump)
                    largestJump = jump;
            }

            AM_EXPECT(largestJump < 0.5f);
        }
    };

    AM_REGISTER_TEST(dsp_resamplers, default_resampler_state_is_continuous_across_blocks);
} // namespace SparkyStudios::Audio::Amplitude::Tests
