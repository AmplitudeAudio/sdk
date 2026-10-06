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

        AmReal64 ZeroCrossings(const std::vector<AmReal32>& x, std::size_t from)
        {
            AmReal64 n = 0;
            for (std::size_t i = from + 1; i < x.size(); ++i)
                if ((x[i - 1] < 0.0f) != (x[i] < 0.0f))
                    ++n;
            return n;
        }

        AmReal64 RMS(const std::vector<AmReal32>& x, std::size_t from)
        {
            AmReal64 sum = 0.0;
            AmSize count = 0;
            for (std::size_t i = from; i < x.size(); ++i)
            {
                sum += static_cast<AmReal64>(x[i]) * static_cast<AmReal64>(x[i]);
                ++count;
            }

            return std::sqrt(sum / static_cast<AmReal64>(count));
        }
    } // namespace

    AM_TEST_CASE(DSPTestCase, dsp_resamplers, resampler_extreme_ratios)
    {
    public:
        void Run() override
        {
            for (const char* name : kResamplerPresets)
            {
                // A stretched kernel cuts at 1/stretch of Nyquist but must still have a DC gain of 1: the level
                // survives the stretch even though the zero crossings pin the pitch down on their own.
                const AmReal64 kToneRms = 0.5 / std::sqrt(2.0);
                constexpr AmReal64 kLevelTolerance = 0.2;

                // Above the stretch cap: the pitch stays exact. 100 Hz at a ratio of 6 is 600 Hz.
                {
                    const std::vector<AmReal32> source = Sine(48000 * 7, 100.0, 48000.0);
                    auto r = Resampler::Construct(name);
                    r->Initialize(1, 48000, 48000);
                    r->SetRatio(6.0);
                    const std::vector<AmReal32> out = RunBlocks(*r, source, 512, 48000);
                    AM_EXPECT_EQ(std::size_t{ 48000 }, out.size());
                    AM_EXPECT(std::abs(ZeroCrossings(out, 1000) - 2.0 * 600.0 * 47000.0 / 48000.0) <= 4.0);
                    AM_EXPECT(std::abs(RMS(out, 1000) - kToneRms) <= kToneRms * kLevelTolerance);
                }

                // Heavy upsampling: 1 kHz at a ratio of 0.01 is 10 Hz.
                {
                    const std::vector<AmReal32> source = Sine(2000, 1000.0, 48000.0);
                    auto r = Resampler::Construct(name);
                    r->Initialize(1, 48000, 48000);
                    r->SetRatio(0.01);
                    const std::vector<AmReal32> out = RunBlocks(*r, source, 512, 96000);
                    AM_EXPECT_EQ(std::size_t{ 96000 }, out.size());
                    AM_EXPECT(std::abs(ZeroCrossings(out, 4800) - 2.0 * 10.0 * 91200.0 / 48000.0) <= 2.0);
                    AM_EXPECT(std::abs(RMS(out, 4800) - kToneRms) <= kToneRms * kLevelTolerance);
                }
            }
        }
    };

    AM_REGISTER_TEST(dsp_resamplers, resampler_extreme_ratios);
} // namespace SparkyStudios::Audio::Amplitude::Tests
