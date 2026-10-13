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

    AM_TEST_CASE(DSPTestCase, dsp_resamplers, resampler_identity_is_bit_exact)
    {
    public:
        void Run() override
        {
            const std::vector<AmReal32> source = Sine(4096, 997.0, 48000.0);
            for (const char* name : kResamplerPresets)
            {
                auto r = Resampler::Construct(name);
                r->Initialize(1, 48000, 48000);
                const std::vector<AmReal32> out = RunBlocks(*r, source, 333, 4000);
                AM_EXPECT_EQ(std::size_t{ 4000 }, out.size());
                for (std::size_t i = 0; i < out.size(); ++i)
                    AM_EXPECT_EQ(source[i], out[i]);
            }
        }
    };

    AM_REGISTER_TEST(dsp_resamplers, resampler_identity_is_bit_exact);
} // namespace SparkyStudios::Audio::Amplitude::Tests
