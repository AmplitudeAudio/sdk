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
    AM_TEST_CASE(DSPTestCase, dsp_resamplers, resampler_never_allocates)
    {
    public:
        void Run() override
        {
#if !defined(AM_NO_MEMORY_STATS)
            // Counts every allocation made through the Amplitude memory pools. The per-frame path touches only the
            // buffers built by Initialize(), so nothing here goes through a pool: SetRatio(), GetInputFramesNeeded(),
            // Process() and Reset() must all run on preallocated storage.
            const auto allocations = []()
            {
                AmUInt64 total = 0;
                for (AmUInt32 k = 0; k < eMemoryPoolKind_COUNT; ++k)
                    total += amMemory->GetStats(static_cast<eMemoryPoolKind>(k)).allocCount.load();
                return total;
            };

            for (const char* name : kResamplerPresets)
            {
                auto r = Resampler::Construct(name);
                r->Initialize(2, 44100, 48000);
                AudioBuffer in(4096, 2);
                AudioBuffer out(512, 2);

                const AmUInt64 before = allocations();
                for (AmUInt32 i = 0; i < 1000; ++i)
                {
                    r->SetRatio(0.5 + static_cast<AmReal64>(i % 70) * 0.05);
                    AmUInt64 inFrames = r->GetInputFramesNeeded(512);
                    AM_EXPECT(inFrames <= in.GetFrameCount());
                    AmUInt64 outFrames = 512;
                    r->Process(in, inFrames, out, outFrames);
                    AM_EXPECT_EQ(512ULL, outFrames);
                    if (i % 100 == 0)
                        r->Reset();
                }

                AM_EXPECT_EQ(before, allocations());
            }
#else
            // The pool counters do not exist in a release build, so this would otherwise register a green test
            // that asserted nothing. Say so rather than pass quietly.
            AM_EXPECT(false);
#endif
        }
    };

    AM_REGISTER_TEST(dsp_resamplers, resampler_never_allocates);
} // namespace SparkyStudios::Audio::Amplitude::Tests
