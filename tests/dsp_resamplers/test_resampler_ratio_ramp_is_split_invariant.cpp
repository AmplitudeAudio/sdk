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

#include <algorithm>
#include <cmath>
#include <vector>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include "DSPTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    namespace
    {
        constexpr double kPi = 3.14159265358979323846;

        std::vector<AmReal32> Sine(AmUInt64 frames, AmReal64 hz, AmReal64 rate)
        {
            std::vector<AmReal32> x(frames);
            for (AmUInt64 i = 0; i < frames; ++i)
                x[i] = static_cast<AmReal32>(0.5 * std::sin(2.0 * kPi * hz * static_cast<AmReal64>(i) / rate));
            return x;
        }

        // Pulls @p totalOut output frames in blocks of @p blockSize, feeding each call exactly the input
        // GetInputFramesNeeded() asks for and carrying the rest over, like ResampleStream's FIFO does.
        bool Stream(
            ResamplerInstance& resampler,
            const std::vector<AmReal32>& source,
            AmUInt64 blockSize,
            AmUInt64 totalOut,
            std::vector<AmReal32>& out)
        {
            AmUInt64 read = 0;
            out.clear();

            AudioBuffer in(source.size(), 1);
            AudioBuffer o(blockSize, 1);

            while (out.size() < totalOut)
            {
                const AmUInt64 want = std::min<AmUInt64>(blockSize, totalOut - out.size());
                const AmUInt64 needed = resampler.GetInputFramesNeeded(want);
                if (read + needed > source.size())
                    return false;

                for (AmUInt64 i = 0; i < needed; ++i)
                    in[0][i] = source[read + i];

                AmUInt64 inFrames = needed;
                AmUInt64 outFrames = want;
                if (!resampler.Process(in, inFrames, o, outFrames) || outFrames != want)
                    return false;

                read += inFrames;
                for (AmUInt64 i = 0; i < outFrames; ++i)
                    out.push_back(o[0][i]);
            }

            return true;
        }
    } // namespace

    AM_TEST_CASE(DSPTestCase, dsp_resamplers, resampler_ratio_ramp_is_split_invariant)
    {
    public:
        void Run() override
        {
            // Several Process() calls share one published ramp: each consumes the part of it that follows the
            // frames already produced. The output must not depend on where the block boundaries fell, which is
            // what ResampleStream::Pull's refill loop assumes when it splits a block into segments.
            constexpr AmReal64 kStart = 0.5;
            constexpr AmReal64 kEnd = 2.0;
            constexpr AmUInt64 kTotal = 2048;
            constexpr AmUInt64 kSplits[] = { 1, 7, 256, 512, 1023 };

            for (const char* name : kResamplerPresets)
            {
                const std::vector<AmReal32> source = Sine(kTotal * 4 + 8192, 997.0, 48000.0);

                auto reference = Resampler::Construct(name);
                reference->Initialize(1, 48000, 48000);
                reference->SetRatioRamp(kStart, kEnd, kTotal);
                std::vector<AmReal32> expected;
                AM_EXPECT(Stream(*reference, source, kTotal, kTotal, expected));

                for (const AmUInt64 split : kSplits)
                {
                    auto instance = Resampler::Construct(name);
                    instance->Initialize(1, 48000, 48000);
                    instance->SetRatioRamp(kStart, kEnd, kTotal);
                    std::vector<AmReal32> actual;
                    AM_EXPECT(Stream(*instance, source, split, kTotal, actual));

                    AM_EXPECT_EQ(expected.size(), actual.size());
                    AM_EXPECT_EQ(static_cast<AmSize>(kTotal), actual.size());

                    // Bit-identical, not merely close: the ramp is in fixed point for exactly this reason.
                    for (AmSize i = 0; i < std::min(expected.size(), actual.size()); ++i)
                        AM_EXPECT(expected[i] == actual[i]);
                }
            }
        }
    };

    AM_REGISTER_TEST(dsp_resamplers, resampler_ratio_ramp_is_split_invariant);
} // namespace SparkyStudios::Audio::Amplitude::Tests