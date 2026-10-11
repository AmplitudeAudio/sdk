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

#include "DSPTestCase.h"
#include "TestRegistry.h"
#include "VoiceSwapTestUtils.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(DSPTestCase, mixer_streaming, voice_two_stream_seeks_in_one_crossfade)
    {
    public:
        void Run() override
        {
            constexpr AmUInt64 kLength = 48000;
            constexpr AmUInt64 kFirst = 1024;
            constexpr AmUInt64 kSecond = kFirst + 100; // Before the crossfade midpoint (192 frames).
            constexpr AmUInt64 kPosition2 = 10000;
            constexpr AmUInt64 kPosition3 = 30000;

            auto s1 = MakeSwapStream(kLength, 0, 0);
            Voice voice;
            AM_EXPECT(voice.Initialize(MakeSwapSettings(*s1, kLength)));
            voice.BindStream(s1.get());

            const AmUInt64 wanted = voice.GetPrerollFrames();
            auto s2 = MakeSwapStream(kLength, kPosition2, wanted);
            auto s3 = MakeSwapStream(kLength, kPosition3, wanted);

            const auto run = RunVoice(voice, 4096, kSwapBlock, [&](AmUInt64 clock) {
                if (clock == kFirst)
                {
                    voice.Enqueue(MakeStreamSeek(kFirst, kPosition2, s2.get()));
                    voice.Enqueue(MakeStreamSeek(kSecond, kPosition3, s3.get()));
                }
            });

            // s2 never became primary and s1 was replaced: each dropped once; s3 (current) never.
            AM_EXPECT_EQ(static_cast<AmSize>(2), EventsOf(run, eVoiceEventKind::StreamDropped).size());
            AM_EXPECT_EQ(static_cast<AmSize>(1), CountDropped(run, s1.get()));
            AM_EXPECT_EQ(static_cast<AmSize>(1), CountDropped(run, s2.get()));
            AM_EXPECT_EQ(static_cast<AmSize>(0), CountDropped(run, s3.get()));

            const auto swapped = EventsOf(run, eVoiceEventKind::StreamSwapped);
            AM_EXPECT_EQ(static_cast<AmSize>(1), swapped.size());
            if (swapped.size() == 1)
                AM_EXPECT(swapped[0].stream == s3.get());

            AM_EXPECT(FindEvent(run, eVoiceEventKind::Underrun) == nullptr);

            for (AmUInt64 i = kSecond + 400; i < run.source.size(); ++i)
                AM_EXPECT_EQ(StreamTestDecoder::ValueAt(kPosition3 + (i - kSecond), 0), run.source[i]);
        }
    };

    AM_REGISTER_TEST(mixer_streaming, voice_two_stream_seeks_in_one_crossfade);
} // namespace SparkyStudios::Audio::Amplitude::Tests
