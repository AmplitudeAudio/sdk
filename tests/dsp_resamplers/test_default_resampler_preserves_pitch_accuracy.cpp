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
    AM_TEST_CASE(DSPTestCase, dsp_resamplers, default_resampler_preserves_pitch_accuracy)
    {
    public:
        void Run() override
        {
            // 44056 Hz is the rate whose exact ratio (6000 / 5507) does not fit the filter budget, so it is
            // snapped to 1071 / 983. The measured pitch error must stay far below anything audible.
            constexpr AmUInt16 channelCount = 1;
            constexpr AmUInt32 sampleRateIn = 44056;
            constexpr AmUInt32 sampleRateOut = 48000;
            constexpr AmUInt64 blockFrames = 4096;
            constexpr AmUInt32 blockCount = 4;
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

            // Measure the output frequency by counting upward zero crossings, skipping the filter warm-up.
            constexpr AmSize warmUpFrames = 256;
            AM_EXPECT(resampled.size() > warmUpFrames + 1024);

            AmSize crossings = 0;
            for (AmSize i = warmUpFrames + 1; i < resampled.size(); ++i)
            {
                if (resampled[i - 1] <= 0.0f && resampled[i] > 0.0f)
                    ++crossings;
            }

            const AmSize measuredFrames = resampled.size() - warmUpFrames - 1;
            const AmReal64 measuredFrequency =
                static_cast<AmReal64>(crossings) * static_cast<AmReal64>(sampleRateOut) / static_cast<AmReal64>(measuredFrames);

            // Zero-crossing counting is granular, so allow one percent; a broken ratio shifts pitch far more.
            AM_EXPECT(std::abs(measuredFrequency - frequency) < 10.0);
        }
    };

    AM_REGISTER_TEST(dsp_resamplers, default_resampler_preserves_pitch_accuracy);
} // namespace SparkyStudios::Audio::Amplitude::Tests
