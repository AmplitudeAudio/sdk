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

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Mixer/Streaming/StreamSource.h>
#include <Mixer/Voice/SourceReader.h>

#include "../mixer_streaming/StreamTestDecoder.h"
#include "ComponentTestCase.h"
#include "TestRegistry.h"
#include "VoiceTestUtils.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    namespace
    {
        constexpr AmUInt64 kLength = 12;

        StreamPlan MakePlan(bool loop)
        {
            StreamPlan plan;
            plan.regionStart = 0;
            plan.regionEnd = kLength;
            plan.loop = loop;
            plan.remainingWraps = loop ? SourceReader::kInfiniteWraps : 0;
            plan.start = 0;
            plan.target = 0;
            return plan;
        }

        VoiceSource MakeSource(StreamSource& stream)
        {
            VoiceSource source;
            source.stream = &stream;
            source.length = kLength;
            source.channels = 1;
            source.sampleRate = 48000;
            return source;
        }
    } // namespace

    AM_TEST_CASE(ComponentTestCase, mixer_voice, source_reader_reads_streams)
    {
    public:
        void Run() override
        {
            AudioBuffer out(40, 1);

            // A non-looping stream ends at its length, read in chunks; a synchronous source fills inline.
            {
                auto decoder = std::make_shared<StreamTestDecoder>(kLength, 1);
                StreamSource stream(decoder, MakePlan(false), 64, 1, 5, 48000, true);

                SourceReader reader;
                reader.Initialize(MakeSource(stream), 0, 0, false, 0);

                SourceReadReport report;
                AmUInt64 delivered = 0;
                for (AmUInt64 done = 0; done < 40; done += 8)
                    delivered += reader.Read(out, done, 8, report);

                AM_EXPECT_EQ(kLength, delivered);
                AM_EXPECT(report.ended);
                AM_EXPECT(!report.underrun);
                AM_EXPECT_EQ(kLength % 8, report.endOffset);
                for (AmUInt64 i = 0; i < kLength; ++i)
                    AM_EXPECT_EQ(StreamTestDecoder::ValueAt(i, 0), out[0][i]);
                for (AmUInt64 i = kLength; i < 40; ++i)
                    AM_EXPECT_EQ(0.0f, out[0][i]);
            }

            // A looping region wraps twice over 30 frames.
            {
                auto decoder = std::make_shared<StreamTestDecoder>(kLength, 1);
                StreamSource stream(decoder, MakePlan(true), 64, 1, 5, 48000, true);

                SourceReader reader;
                reader.Initialize(MakeSource(stream), 0, 0, true, 0);

                SourceReadReport report;
                AM_EXPECT_EQ(30ULL, reader.Read(out, 0, 30, report));
                AM_EXPECT_EQ(2U, report.wraps);
                for (AmUInt64 i = 0; i < 30; ++i)
                    AM_EXPECT_EQ(StreamTestDecoder::ValueAt(i % kLength, 0), out[0][i]);
            }
        }
    };

    AM_REGISTER_TEST(mixer_voice, source_reader_reads_streams);
} // namespace SparkyStudios::Audio::Amplitude::Tests
