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

#include <limits>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <DSP/Resamplers/BandlimitedResampler.h>

#include "DSPTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    // The playback ratio reaches SetRatio() from RTPC values and from rate arithmetic the caller cannot fully
    // control, so a zero, a negative, a NaN or an absurd magnitude has to arrive as a usable 1:1 pass-through
    // rather than as a divide by zero or a read-ahead sized for the whole stream.
    AM_TEST_CASE(DSPTestCase, dsp_resamplers, default_resampler_set_ratio)
    {
    public:
        void Run() override
        {
            constexpr AmUInt16 channelCount = 1;
            constexpr AmUInt32 sampleRateIn = 48000;
            constexpr AmUInt32 sampleRateOut = 48000;
            constexpr AmUInt64 blockFrames = 512;

            const double hostile[] = {
                0.0,
                -1.0,
                std::numeric_limits<double>::infinity(),
                std::numeric_limits<double>::quiet_NaN(),
            };

            // Wider than kMaxRatio: this one is meant to be clamped, not reset, so it belongs outside the loop
            // that pins the hostile values onto 1:1.
            constexpr double kAbsurdlyWide = 1e9;

            for (const char* name : kResamplerPresets)
            {
                auto resampler = Resampler::Construct(name);
                resampler->Initialize(channelCount, sampleRateIn, sampleRateOut);

                // Reset() first: CurrentReach() only collapses to zero at a whole-frame phase, and the read-ahead
                // below is read at the same phase every time so that it means the same thing each iteration.
                resampler->Reset();
                resampler->SetRatio(1.0);
                const AmUInt64 passThrough = resampler->GetInputFramesNeeded(blockFrames);
                AM_EXPECT(passThrough > 0);

                for (const double ratio : hostile)
                {
                    resampler->Reset();
                    resampler->SetRatio(ratio);

                    // Exactly 1:1: the clamping path would also leave a ratio that fills a block.
                    AM_EXPECT_EQ(passThrough, resampler->GetInputFramesNeeded(blockFrames));

                    const AmUInt64 needed = resampler->GetInputFramesNeeded(blockFrames);
                    AM_EXPECT(needed > 0);

                    AudioBuffer inputBuffer(needed, channelCount);
                    GenerateSineWave(inputBuffer, sampleRateIn);

                    AudioBuffer outputBuffer(blockFrames, channelCount);
                    AmUInt64 processedInputFrames = needed;
                    AmUInt64 processedOutputFrames = blockFrames;

                    AM_EXPECT(resampler->Process(inputBuffer, processedInputFrames, outputBuffer, processedOutputFrames));

                    // A usable ratio fills a block instead of stalling the stream.
                    AM_EXPECT_EQ(blockFrames, processedOutputFrames);
                }

                // Clamped rather than reset, and still a ratio the stream can be driven at: the input has to cover
                // it, because a 512-frame block at kMaxRatio needs tens of millions of input frames.
                resampler->Reset();
                resampler->SetRatio(kAbsurdlyWide);

                const AmUInt64 needed = resampler->GetInputFramesNeeded(blockFrames);
                AM_EXPECT(needed > passThrough);

                AudioBuffer inputBuffer(needed, channelCount);
                GenerateSineWave(inputBuffer, sampleRateIn);

                AudioBuffer outputBuffer(blockFrames, channelCount);
                AmUInt64 processedInputFrames = needed;
                AmUInt64 processedOutputFrames = blockFrames;

                AM_EXPECT(resampler->Process(inputBuffer, processedInputFrames, outputBuffer, processedOutputFrames));
                AM_EXPECT_EQ(blockFrames, processedOutputFrames);
            }
        }
    };

    AM_REGISTER_TEST(dsp_resamplers, default_resampler_set_ratio);
} // namespace SparkyStudios::Audio::Amplitude::Tests