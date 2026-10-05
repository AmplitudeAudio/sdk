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

#include <DSP/Resamplers/BandlimitedResampler.h>

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
            // Upsampling 8000 Hz to 48000 Hz in 8-frame blocks: the 80-tap kernel reads 40 frames ahead, so every
            // Process() call consumes only part of what it is given and the rest is carried into the next one. Only a
            // correct history carry keeps the output continuous across the seams.
            constexpr AmUInt16 channelCount = 1;
            constexpr AmUInt32 sampleRateIn = 8000;
            constexpr AmUInt32 sampleRateOut = 48000;
            constexpr AmUInt64 blockFrames = 8;
            constexpr AmUInt32 blockCount = 64;
            constexpr AmReal32 frequency = 200.0f;

            auto resampler = std::make_shared<BandlimitedResampler>("default", eResamplerPreset::SincBest);
            auto instance = resampler->CreateInstance();

            instance->Initialize(channelCount, sampleRateIn, sampleRateOut);

            std::vector<AmReal32> resampled;
            std::vector<AmReal32> pending;
            AmUInt64 phase = 0;

            for (AmUInt32 block = 0; block < blockCount; ++block)
            {
                for (AmUInt64 i = 0; i < blockFrames; ++i, ++phase)
                    pending.push_back(
                        std::sin(2.0f * AM_PI32 * frequency * static_cast<AmReal32>(phase) / static_cast<AmReal32>(sampleRateIn)));

                while (!pending.empty())
                {
                    AudioBuffer inputBuffer(pending.size(), channelCount);
                    std::copy(pending.begin(), pending.end(), inputBuffer[0].begin());

                    const AmUInt64 capacity = pending.size() * sampleRateOut / sampleRateIn + 2;
                    AudioBuffer outputBuffer(capacity, channelCount);

                    AmUInt64 processedInputFrames = pending.size();
                    AmUInt64 processedOutputFrames = capacity;

                    AM_EXPECT(instance->Process(inputBuffer, processedInputFrames, outputBuffer, processedOutputFrames));

                    for (AmUInt64 i = 0; i < processedOutputFrames; ++i)
                        resampled.push_back(outputBuffer[0][i]);

                    if (processedInputFrames == 0 && processedOutputFrames == 0)
                        break;

                    pending.erase(pending.begin(), pending.begin() + static_cast<std::ptrdiff_t>(processedInputFrames));
                }
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
