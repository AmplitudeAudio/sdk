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
    AM_TEST_CASE(ComponentTestCase, mixer_streaming, stream_source_fills_in_play_order)
    {
    public:
        void Run() override
        {
            auto decoder = std::make_shared<StreamTestDecoder>(100, 2);

            StreamPlan plan;
            plan.regionStart = 10;
            plan.regionEnd = 40;
            plan.loop = true;
            plan.remainingWraps = SourceReader::kInfiniteWraps;
            plan.start = 25;
            plan.target = 25;
            plan.targetIndex = 0;

            StreamSource source(decoder, plan, 64, 2, 16, 48000, false);

            AM_EXPECT_EQ(64u, source.Fill(1000));
            AM_EXPECT_EQ(64u, source.GetBufferedFrames());
            AM_EXPECT(!source.IsFilled());

            // 25..39, then 10..39, then 10..28
            std::vector<AmUInt64> expected;
            for (AmUInt64 f = 25; f < 40; ++f)
                expected.push_back(f);
            for (AmUInt64 f = 10; f < 40; ++f)
                expected.push_back(f);
            for (AmUInt64 f = 10; f < 29; ++f)
                expected.push_back(f);
            AM_EXPECT_EQ(64u, expected.size());

            AudioBuffer out(64, 2);
            AM_EXPECT_EQ(64u, source.GetRing().Read(0, out, 0, 64));
            for (AmUInt64 i = 0; i < 64; ++i)
            {
                AM_EXPECT_EQ(StreamTestDecoder::ValueAt(expected[i], 0), out[0][i]);
                AM_EXPECT_EQ(StreamTestDecoder::ValueAt(expected[i], 1), out[1][i]);
            }

            AM_EXPECT_EQ(10u, source.SourceFrameOf(15));
            AM_EXPECT_EQ(10u, source.SourceFrameOf(45));

            AmUInt64 index = 0;
            AM_EXPECT(source.IndexOf(30, index));
            AM_EXPECT_EQ(5u, index);
            AM_EXPECT(source.IndexOf(12, index));
            AM_EXPECT_EQ(17u, index);
            AM_EXPECT(!source.IndexOf(45, index));

            // Releasing space lets the sequence continue: 29..39, then 10..30.
            source.GetRing().Release(32);
            AM_EXPECT_EQ(32u, source.Fill(1000));
            AudioBuffer more(32, 2);
            AM_EXPECT_EQ(32u, source.GetRing().Read(64, more, 0, 32));
            for (AmUInt64 i = 0; i < 32; ++i)
                AM_EXPECT_EQ(StreamTestDecoder::ValueAt(source.SourceFrameOf(64 + i), 0), more[0][i]);
            AM_EXPECT_EQ(StreamTestDecoder::ValueAt(29, 0), more[0][0]);
        }
    };

    AM_REGISTER_TEST(mixer_streaming, stream_source_fills_in_play_order);
} // namespace SparkyStudios::Audio::Amplitude::Tests
