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
    AM_TEST_CASE(DSPTestCase, dsp_resamplers, resampler_long_run_does_not_drift)
    {
    public:
        void Run() override
        {
            // Ten million output frames at 44.1 kHz -> 48 kHz (about 3.5 minutes): the frames consumed match the exact
            // ratio to within one frame.
            auto r = Resampler::Construct("linear");
            r->Initialize(1, 44100, 48000);

            constexpr AmUInt64 kTotal = 10000000;
            constexpr AmUInt64 kBlock = 4096;
            AudioBuffer in(8192, 1);
            AudioBuffer out(kBlock, 1);
            AmUInt64 consumed = 0;
            AmUInt64 producedTotal = 0;
            for (producedTotal = 0; producedTotal < kTotal; producedTotal += kBlock)
            {
                AmUInt64 inFrames = r->GetInputFramesNeeded(kBlock);
                AM_EXPECT(inFrames <= in.GetFrameCount());
                AmUInt64 outFrames = kBlock;
                r->Process(in, inFrames, out, outFrames);
                AM_EXPECT_EQ(kBlock, outFrames);
                consumed += inFrames;
            }

            // The loop runs in whole blocks, so it always overshoots kTotal by less than one block: compare the
            // frames consumed against the frames actually produced, not against kTotal.
            const AmReal64 exact = static_cast<AmReal64>(producedTotal) * 44100.0 / 48000.0;
            AM_EXPECT(std::abs(static_cast<AmReal64>(consumed) - exact) <= 1.0);

            // And to the frame: the accumulator is 32.32 fixed point, so the input consumed is exactly the frames
            // produced times the quantised step, with nothing rounded along the way. A float ratio accumulated per
            // block drifts by a fraction of a frame over this run and misses this.
            const auto step = static_cast<AmUInt64>(std::llround(44100.0 / 48000.0 * 4294967296.0));
            AM_EXPECT_EQ((producedTotal * step) >> 32, consumed);
        }
    };

    AM_REGISTER_TEST(dsp_resamplers, resampler_long_run_does_not_drift);
} // namespace SparkyStudios::Audio::Amplitude::Tests
