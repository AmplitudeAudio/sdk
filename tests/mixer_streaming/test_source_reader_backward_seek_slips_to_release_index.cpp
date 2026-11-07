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

    AM_TEST_CASE(ComponentTestCase, mixer_streaming, source_reader_backward_seek_slips_to_release_index)
    {
    public:
        void Run() override
        {
            auto decoder = std::make_shared<StreamTestDecoder>(100, 1);
            StreamSource stream(decoder, MakePlan(), 64, 1, 8, 48000, false);
            stream.Fill(60);

            SourceReader reader;
            reader.Initialize(MakeStreamSource(stream), 0, 100, false, 0);

            AudioBuffer out(50, 1);
            SourceReadReport report;
            AM_EXPECT_EQ(50ULL, reader.Read(out, 0, 50, report));
            AM_EXPECT_EQ(50ULL, stream.GetRing().GetReleaseIndex());

            // A backward seek inside the released frames slips forward to the oldest frame the ring still holds.
            reader.Seek(10);
            AM_EXPECT_EQ(50ULL, reader.GetCursor());

            report = {};
            AM_EXPECT_EQ(10ULL, reader.Read(out, 0, 10, report));
            AM_EXPECT(!report.underrun);
            for (AmUInt64 i = 0; i < 10; ++i)
                AM_EXPECT_EQ(StreamTestDecoder::ValueAt(50 + i, 0), out[0][i]);
        }
    };

    AM_REGISTER_TEST(mixer_streaming, source_reader_backward_seek_slips_to_release_index);
} // namespace SparkyStudios::Audio::Amplitude::Tests
