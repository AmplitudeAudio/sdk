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

#include <chrono>
#include <future>
#include <vector>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Mixer/Streaming/StreamSource.h>
#include <Mixer/Voice/SourceReader.h>

#include "ComponentTestCase.h"
#include "StreamTestDecoder.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(ComponentTestCase, mixer_streaming, stream_source_plans_preroll)
    {
    public:
        void Run() override
        {
            constexpr AmUInt32 inf = SourceReader::kInfiniteWraps;
            AmUInt64 pre = 99;
            AmUInt64 index = 0;

            // Preroll comes from before the target.
            {
                const StreamPlan plan = PlanStream(0, 100, false, 0, 40, 16, pre);
                AM_EXPECT_EQ(24u, plan.start);
                AM_EXPECT_EQ(16u, pre);
                AM_EXPECT_EQ(16u, plan.targetIndex);
                AM_EXPECT_EQ(40u, plan.target);

                StreamSource source(std::make_shared<StreamTestDecoder>(100, 1), plan, 64, 1, 16, 48000, false);
                AM_EXPECT(source.IndexOf(40, index));
                AM_EXPECT_EQ(16u, index);
            }

            // Preroll wraps to the loop tail.
            {
                const StreamPlan plan = PlanStream(0, 100, true, inf, 5, 16, pre);
                AM_EXPECT_EQ(89u, plan.start);
                AM_EXPECT_EQ(16u, pre);
                AM_EXPECT_EQ(inf, plan.remainingWraps);
            }

            // Not enough played frames for the full preroll.
            {
                const StreamPlan plan = PlanStream(0, 100, false, 0, 5, 16, pre);
                AM_EXPECT_EQ(0u, plan.start);
                AM_EXPECT_EQ(5u, pre);
            }

            // Target at the region start: no preroll.
            {
                const StreamPlan plan = PlanStream(0, 100, true, inf, 0, 16, pre);
                AM_EXPECT_EQ(0u, plan.start);
                AM_EXPECT_EQ(0u, pre);
                AM_EXPECT_EQ(0u, plan.targetIndex);
            }

            // A loop shorter than the preroll: capped to one pass, and the target index wins over the first occurrence.
            {
                const StreamPlan plan = PlanStream(5, 8, true, inf, 6, 16, pre);
                AM_EXPECT_EQ(3u, pre);
                AM_EXPECT_EQ(6u, plan.start);

                StreamSource source(std::make_shared<StreamTestDecoder>(100, 1), plan, 32, 1, 16, 48000, false);
                AM_EXPECT(source.IndexOf(6, index));
                AM_EXPECT_EQ(3u, index);

                source.Fill(32);
                AudioBuffer out(1, 1);
                AM_EXPECT_EQ(1u, source.GetRing().Read(3, out, 0, 1));
                AM_EXPECT_EQ(StreamTestDecoder::ValueAt(6, 0), out[0][0]);
            }

            // A finite loop gets one more wrap for the tail played before the wrap to the target.
            {
                const StreamPlan plan = PlanStream(0, 100, true, 0, 2, 16, pre);
                AM_EXPECT_EQ(1u, plan.remainingWraps);
            }
        }
    };

    AM_REGISTER_TEST(mixer_streaming, stream_source_plans_preroll);
} // namespace SparkyStudios::Audio::Amplitude::Tests
