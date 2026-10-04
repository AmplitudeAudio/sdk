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
        std::vector<AmReal32> Sine(AmSize frames, AmReal64 hz, AmReal64 rate)
        {
            std::vector<AmReal32> x(frames);
            for (AmSize i = 0; i < frames; ++i)
                x[i] = static_cast<AmReal32>(0.5 * std::sin(2.0 * 3.14159265358979323846 * hz * static_cast<AmReal64>(i) / rate));
            return x;
        }

        // Pulls totalOut frames in blocks of blockSize, feeding exactly GetInputFramesNeeded() frames each call and
        // re-feeding whatever was not consumed, like a FIFO would.
        bool Stream(
            ResamplerInstance& r, const std::vector<AmReal32>& source, AmUInt64 blockSize, AmUInt64 totalOut, std::vector<AmReal32>& out)
        {
            AmUInt64 read = 0;
            out.clear();

            AudioBuffer in(source.size(), 1);
            AudioBuffer o(blockSize, 1);

            while (out.size() < totalOut)
            {
                const AmUInt64 want = std::min<AmUInt64>(blockSize, totalOut - out.size());
                const AmUInt64 needed = r.GetInputFramesNeeded(want);
                if (read + needed > source.size())
                    return false;

                for (AmUInt64 i = 0; i < needed; ++i)
                    in[0][i] = source[read + i];

                AmUInt64 inFrames = needed;
                AmUInt64 outFrames = want;
                if (!r.Process(in, inFrames, o, outFrames) || outFrames != want)
                    return false;

                read += inFrames;
                for (AmUInt64 i = 0; i < outFrames; ++i)
                    out.push_back(o[0][i]);
            }

            return true;
        }
    } // namespace

    AM_TEST_CASE(DSPTestCase, dsp_resamplers, resampler_contract_split_invariance)
    {
    public:
        void Run() override
        {
            constexpr AmUInt32 kRates[][2] = { { 44100, 48000 }, { 48000, 44100 }, { 22050, 48000 }, { 96000, 48000 } };
            constexpr AmUInt64 kBlocks[] = { 1, 7, 256, 1023 };
            constexpr AmUInt64 kTotal = 8192;

            for (const char* name : kResamplerPresets)
            {
                for (const auto& rates : kRates)
                {
                    const std::vector<AmReal32> source = Sine(kTotal * 3, 997.0, rates[0]);

                    auto reference = Resampler::Construct(name);
                    reference->Initialize(1, rates[0], rates[1]);
                    std::vector<AmReal32> expected;
                    AM_EXPECT(Stream(*reference, source, kTotal, kTotal, expected));

                    for (const AmUInt64 block : kBlocks)
                    {
                        auto instance = Resampler::Construct(name);
                        instance->Initialize(1, rates[0], rates[1]);
                        std::vector<AmReal32> actual;
                        AM_EXPECT(Stream(*instance, source, block, kTotal, actual));

                        AM_EXPECT_EQ(expected.size(), actual.size());

                        AmReal32 worst = 0.0f;
                        const AmSize compared = std::min<AmSize>(kTotal, std::min(expected.size(), actual.size()));
                        for (AmSize i = 0; i < compared; ++i)
                            worst = std::max(worst, std::abs(actual[i] - expected[i]));

                        AM_EXPECT(compared == kTotal);
                        AM_EXPECT(worst < 1e-6f);
                    }
                }
            }
        }
    };

    AM_REGISTER_TEST(dsp_resamplers, resampler_contract_split_invariance);
} // namespace SparkyStudios::Audio::Amplitude::Tests
