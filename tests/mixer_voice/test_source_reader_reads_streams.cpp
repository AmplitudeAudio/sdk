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

#include <Mixer/Voice/SourceReader.h>

#include "ComponentTestCase.h"
#include "TestRegistry.h"
#include "VoiceTestUtils.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    namespace
    {
        struct FakeStream
        {
            AudioBuffer buffer{ 7, 1 };  // decode capacity of 7 frames
            AmUInt64 length = 20;
            AmUInt64 dryAfter = 1000;    // returns nothing past this frame
        };

        AmUInt64 Decode(void* context, AmUInt64 offset, AmUInt64 frames)
        {
            auto* stream = static_cast<FakeStream*>(context);
            AmUInt64 n = 0;
            for (; n < frames && offset + n < stream->length && offset + n < stream->dryAfter; ++n)
                stream->buffer[0][n] = static_cast<AmReal32>(offset + n);
            return n;
        }
    } // namespace

    AM_TEST_CASE(ComponentTestCase, mixer_voice, source_reader_reads_streams)
    {
    public:
        void Run() override
        {
            FakeStream stream;
            VoiceSource source;
            source.buffer = &stream.buffer;
            source.decode = &Decode;
            source.context = &stream;
            source.decodeCapacity = 7;
            source.length = 20;
            source.channels = 1;
            source.sampleRate = 48000;

            SourceReader reader;
            reader.Initialize(source, 0, 0, true, 0);

            AudioBuffer out(45, 1);
            SourceReadReport report;
            AM_EXPECT_EQ(45ULL, reader.Read(out, 0, 45, report));
            AM_EXPECT_EQ(2U, report.wraps);
            for (AmUInt64 i = 0; i < 45; ++i)
                AM_EXPECT_EQ(static_cast<AmReal32>(i % 20), out[0][i]);

            // A stream that runs dry before its end is treated as ended.
            stream.dryAfter = 12;
            SourceReader starved;
            starved.Initialize(source, 0, 0, false, 0);
            report = {};
            AM_EXPECT_EQ(12ULL, starved.Read(out, 0, 45, report));
            AM_EXPECT(report.starved);
            AM_EXPECT(report.ended);
            AM_EXPECT_EQ(12ULL, report.endOffset);
        }
    };

    AM_REGISTER_TEST(mixer_voice, source_reader_reads_streams);
} // namespace SparkyStudios::Audio::Amplitude::Tests
