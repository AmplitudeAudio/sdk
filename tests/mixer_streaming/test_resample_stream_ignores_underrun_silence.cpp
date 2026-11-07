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
#include <Mixer/Voice/ResampleStream.h>

#include "DSPTestCase.h"
#include "StreamTestDecoder.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(DSPTestCase, mixer_streaming, resample_stream_ignores_underrun_silence)
    {
    public:
        void Run() override
        {
            constexpr AmUInt64 kFrames = 1000;

            StreamPlan plan;
            plan.regionStart = 0;
            plan.regionEnd = kFrames;
            plan.start = 0;
            plan.target = 0;

            auto decoder = std::make_shared<StreamTestDecoder>(kFrames, 1);
            StreamSource stream(decoder, plan, 1024, 1, 64, 48000, false);
            stream.Fill(100);

            VoiceSource source;
            source.stream = &stream;
            source.length = kFrames;
            source.channels = 1;
            source.sampleRate = 48000;

            SourceReader reader;
            reader.Initialize(source, 0, kFrames, false, 0);

            ResampleStream resample;
            AM_EXPECT(resample.Initialize("linear", 48000, 48000, 1, 256));

            AudioBuffer out(256, 1);
            const auto first = resample.Pull(reader, out, 0, 256);
            AM_EXPECT(first.underrun);
            AM_EXPECT(!first.error);
            AM_EXPECT_EQ(256ULL, first.produced);

            // Output stops at the frames the stream delivered; the rest is silence.
            AmUInt64 audible = 0;
            while (audible < 256 && out[0][audible] != 0.0f)
                ++audible;
            AM_EXPECT(audible > 0 && audible <= 100);
            for (AmUInt64 i = audible; i < 256; ++i)
                AM_EXPECT_EQ(0.0f, out[0][i]);

            // The position is the source frames actually consumed, not the block length.
            const AmUInt64 position = resample.GetSourcePosition(reader);
            AM_EXPECT(position <= 100);
            AM_EXPECT(audible <= position + 1);

            // Once the ring is refilled, the output continues the ramp with no skipped frame.
            stream.Fill(300);
            const auto second = resample.Pull(reader, out, 0, 256);
            AM_EXPECT(!second.error);
            AM_EXPECT_EQ(256ULL, second.produced);

            AM_EXPECT(out[0][0] != 0.0f);
            const AmReal32 expectedFirst = StreamTestDecoder::ValueAt(audible, 0);
            AM_EXPECT(std::abs(out[0][0] - expectedFirst) <= 1.5f);
            for (AmUInt64 i = 1; i < 256; ++i)
                AM_EXPECT(std::abs((out[0][i] - out[0][i - 1]) - 1.0f) < 0.01f);
        }
    };

    AM_REGISTER_TEST(mixer_streaming, resample_stream_ignores_underrun_silence);
} // namespace SparkyStudios::Audio::Amplitude::Tests
