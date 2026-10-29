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

#include "ComponentTestCase.h"
#include "StreamTestDecoder.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    namespace
    {
        VoiceSource MakeStreamSource(StreamSource& stream)
        {
            VoiceSource source;
            source.stream = &stream;
            source.length = 100;
            source.channels = 1;
            source.sampleRate = 48000;
            return source;
        }

        StreamPlan MakePlan()
        {
            StreamPlan plan;
            plan.regionStart = 0;
            plan.regionEnd = 100;
            plan.start = 0;
            plan.target = 0;
            return plan;
        }
    } // namespace

    AM_TEST_CASE(ComponentTestCase, mixer_streaming, source_reader_underrun_holds_cursor)
    {
    public:
        void Run() override
        {
            // An underrun is silence, and the cursor holds.
            {
                auto decoder = std::make_shared<StreamTestDecoder>(100, 1);
                StreamSource stream(decoder, MakePlan(), 64, 1, 8, 48000, false);
                stream.Fill(20);

                SourceReader reader;
                reader.Initialize(MakeStreamSource(stream), 0, 100, false, 0);

                AudioBuffer out(30, 1);
                SourceReadReport report;
                AM_EXPECT_EQ(20ULL, reader.Read(out, 0, 30, report));
                AM_EXPECT(report.underrun);
                AM_EXPECT(!report.ended);
                AM_EXPECT_EQ(20ULL, reader.GetCursor());
                AM_EXPECT(!reader.IsEnded());
                for (AmUInt64 i = 0; i < 20; ++i)
                    AM_EXPECT_EQ(StreamTestDecoder::ValueAt(i, 0), out[0][i]);
                for (AmUInt64 i = 20; i < 30; ++i)
                    AM_EXPECT_EQ(0.0f, out[0][i]);

                // The next read resumes at the held frame.
                stream.Fill(40);
                report = {};
                AM_EXPECT_EQ(30ULL, reader.Read(out, 0, 30, report));
                AM_EXPECT(!report.underrun);
                for (AmUInt64 i = 0; i < 30; ++i)
                    AM_EXPECT_EQ(StreamTestDecoder::ValueAt(20 + i, 0), out[0][i]);

                // The stream's end ends the reader exactly at frame 100.
                AM_EXPECT_EQ(50ULL, reader.GetCursor());
                AM_EXPECT_EQ(50ULL, stream.GetRing().GetReleaseIndex());

                AudioBuffer big(100, 1);
                AmUInt64 total = 50;
                while (!stream.IsFilled())
                {
                    stream.Fill(1000);
                    report = {};
                    total += reader.Read(big, 0, 20, report);
                }

                report = {};
                AmUInt64 delivered = 0;
                while (!report.ended)
                    delivered += reader.Read(big, 0, 30, report);

                AM_EXPECT(report.ended);
                AM_EXPECT(!report.underrun);
                AM_EXPECT_EQ(100ULL, total + delivered);
                AM_EXPECT(reader.IsEnded());
            }

            // A failed stream ends the reader at the failure, not as an underrun.
            {
                auto decoder = std::make_shared<StreamTestDecoder>(100, 1);
                decoder->FailAt(10);
                StreamSource stream(decoder, MakePlan(), 64, 1, 8, 48000, false);
                stream.Fill(1000);

                SourceReader reader;
                reader.Initialize(MakeStreamSource(stream), 0, 100, false, 0);

                AudioBuffer out(30, 1);
                SourceReadReport report;
                AM_EXPECT_EQ(10ULL, reader.Read(out, 0, 30, report));
                AM_EXPECT(report.ended);
                AM_EXPECT(!report.underrun);
                AM_EXPECT_EQ(10ULL, report.endOffset);
            }

            // A read-ahead copy does not release the frames the main reader still needs.
            {
                auto decoder = std::make_shared<StreamTestDecoder>(100, 1);
                StreamSource stream(decoder, MakePlan(), 64, 1, 8, 48000, false);
                stream.Fill(60);

                SourceReader reader;
                reader.Initialize(MakeStreamSource(stream), 0, 100, false, 0);

                AudioBuffer out(60, 1);
                SourceReadReport report;
                AM_EXPECT_EQ(10ULL, reader.Read(out, 0, 10, report));
                AM_EXPECT_EQ(10ULL, stream.GetRing().GetReleaseIndex());

                SourceReader ahead = reader;
                ahead.SetReleasesRing(false);
                AM_EXPECT_EQ(40ULL, ahead.Read(out, 0, 40, report));
                AM_EXPECT_EQ(10ULL, stream.GetRing().GetReleaseIndex());

                AM_EXPECT_EQ(40ULL, reader.Read(out, 0, 40, report));
                AM_EXPECT_EQ(50ULL, stream.GetRing().GetReleaseIndex());
            }
        }
    };

    AM_REGISTER_TEST(mixer_streaming, source_reader_underrun_holds_cursor);
} // namespace SparkyStudios::Audio::Amplitude::Tests
