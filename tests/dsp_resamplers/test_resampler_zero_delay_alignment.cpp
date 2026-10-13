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
#include <functional>
#include <vector>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include "DSPTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    namespace
    {
        std::vector<AmReal32> Sine(AmSize frames, AmReal64 hz, AmReal64 rate, AmReal64 amplitude = 0.5)
        {
            std::vector<AmReal32> x(frames);
            for (AmSize i = 0; i < frames; ++i)
                x[i] = static_cast<AmReal32>(amplitude * std::sin(2.0 * AM_PI * hz * static_cast<AmReal64>(i) / rate));
            return x;
        }

        // Pulls total frames in blocks, feeding GetInputFramesNeeded() frames each call (zeros past the source end).
        // before(block) runs before each block, e.g. to change the ratio.
        std::vector<AmReal32> RunBlocks(
            ResamplerInstance& r,
            const std::vector<AmReal32>& source,
            AmUInt64 block,
            AmUInt64 total,
            const std::function<void(AmUInt64)>& before = nullptr)
        {
            std::vector<AmReal32> out;
            AmUInt64 read = 0;
            AmUInt64 index = 0;
            while (out.size() < total)
            {
                if (before)
                    before(index++);

                const AmUInt64 want = std::min<AmUInt64>(block, total - out.size());
                const AmUInt64 needed = r.GetInputFramesNeeded(want);
                AudioBuffer in(std::max<AmUInt64>(needed, 1), 1);
                for (AmUInt64 i = 0; i < needed; ++i)
                    in[0][i] = read + i < source.size() ? source[read + i] : 0.0f;

                AudioBuffer o(want, 1);
                AmUInt64 inFrames = needed;
                AmUInt64 outFrames = want;
                r.Process(in, inFrames, o, outFrames);
                read += inFrames;
                for (AmUInt64 i = 0; i < outFrames; ++i)
                    out.push_back(o[0][i]);

                if (outFrames == 0 && inFrames == 0)
                    break;
            }

            return out;
        }
    } // namespace

    AM_TEST_CASE(DSPTestCase, dsp_resamplers, resampler_zero_delay_alignment)
    {
    public:
        void Run() override
        {
            constexpr AmUInt64 kImpulse = 400;
            for (const char* name : kResamplerPresets)
            {
                for (const AmReal64 ratio : { 0.5, 44100.0 / 48000.0, 1.0, 2.0, 4.0 })
                {
                    std::vector<AmReal32> source(4096, 0.0f);
                    source[kImpulse] = 1.0f;

                    auto r = Resampler::Construct(name);
                    r->Initialize(1, 48000, 48000);
                    r->SetRatio(ratio);
                    AM_EXPECT_EQ(0ULL, r->GetLatency());

                    const std::vector<AmReal32> out = RunBlocks(*r, source, 256, 1600);
                    AM_EXPECT_EQ(std::size_t{ 1600 }, out.size());

                    std::size_t peak = 0;
                    for (std::size_t i = 1; i < out.size(); ++i)
                        if (std::abs(out[i]) > std::abs(out[peak]))
                            peak = i;

                    // Zero delay: the impulse shows up at input frame / ratio, never later.
                    AM_EXPECT(std::abs(static_cast<AmReal64>(peak) - static_cast<AmReal64>(kImpulse) / ratio) <= 0.5 + 1e-9);
                }
            }
        }
    };

    AM_REGISTER_TEST(dsp_resamplers, resampler_zero_delay_alignment);
} // namespace SparkyStudios::Audio::Amplitude::Tests
