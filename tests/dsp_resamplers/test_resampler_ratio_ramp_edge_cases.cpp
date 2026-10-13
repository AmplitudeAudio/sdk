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
#include <limits>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include "DSPTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(DSPTestCase, dsp_resamplers, resampler_ratio_ramp_edge_cases)
    {
    public:
        void Run() override
        {
            for (const char* name : kResamplerPresets)
            {
                auto made = Resampler::Construct(name);
                auto* r = dynamic_cast<BandlimitedResamplerInstance*>(made.get());
                AM_EXPECT_NOT(r == nullptr);
                if (r == nullptr)
                    continue;

                r->Initialize(1, 48000, 48000);
                AudioBuffer in(8192, 1);
                AudioBuffer out(512, 1);

                // A ramp over no frames is a constant ratio at its end, and reports it.
                r->SetRatioRamp(0.5, 2.0, 0);
                AM_EXPECT(std::abs(r->GetRatio() - 2.0) < 1e-12);

                // A ramp over one frame consumes its mean over that frame.
                r->Reset();
                r->SetRatioRamp(1.0, 3.0, 1);
                AM_EXPECT(std::abs(r->GetRatio() - 2.0) < 1e-12);
                {
                    AmUInt64 inFrames = r->GetInputFramesNeeded(1);
                    AmUInt64 outFrames = 1;
                    r->Process(in, inFrames, out, outFrames);
                    AM_EXPECT_EQ(1ULL, outFrames);
                    AM_EXPECT_EQ(2ULL, inFrames);
                }

                // A spent ramp holds its end ratio, and the read-ahead shrinks back to it: an instance that ramped from
                // 4 down to 1 asks for exactly what one set to 1 at the same phase does.
                r->Reset();
                r->SetRatioRamp(4.0, 1.0, 256);
                {
                    AmUInt64 inFrames = r->GetInputFramesNeeded(256);
                    AmUInt64 outFrames = 256;
                    r->Process(in, inFrames, out, outFrames);
                    AM_EXPECT_EQ(256ULL, outFrames);
                }

                AM_EXPECT(std::abs(r->GetRatio() - 1.0) < 1e-12);
                const AmUInt64 afterRamp = r->GetInputFramesNeeded(64);
                r->SetRatio(1.0);
                AM_EXPECT_EQ(r->GetInputFramesNeeded(64), afterRamp);

                // A huge request saturates instead of wrapping into a small count.
                r->Reset();
                r->SetRatio(1.0 + 1.0 / 3.0);
                {
                    AmUInt64 inFrames = r->GetInputFramesNeeded(7);
                    AmUInt64 outFrames = 7;
                    r->Process(in, inFrames, out, outFrames);
                }

                AM_EXPECT(r->GetInputFramesNeeded(std::numeric_limits<AmUInt64>::max()) > (1ULL << 31));
            }
        }
    };

    AM_REGISTER_TEST(dsp_resamplers, resampler_ratio_ramp_edge_cases);
} // namespace SparkyStudios::Audio::Amplitude::Tests
