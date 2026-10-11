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
    AM_TEST_CASE(DSPTestCase, mixer_streaming, voice_direct_reposition_tail_reports_intermediate_streams)
    {
    public:
        void Run() override
        {
            // The source ends inside the block, before two stream seeks stamped after the end: they reposition directly.
            constexpr AmUInt64 kLength = 1000;
            constexpr AmUInt64 kBlockStart = 768;

            auto s1 = MakeSwapStream(kLength, 0, 0);
            Voice voice;
            AM_EXPECT(voice.Initialize(MakeSwapSettings(*s1, kLength)));
            voice.BindStream(s1.get());

            const AmUInt64 wanted = voice.GetPrerollFrames();
            auto s2 = MakeSwapStream(kLength, 100, wanted);
            auto s3 = MakeSwapStream(kLength, 200, wanted);

            const auto run = RunVoice(voice, 1280, kSwapBlock, [&](AmUInt64 clock) {
                if (clock == kBlockStart)
                {
                    voice.Enqueue(MakeStreamSeek(kBlockStart + 242, 100, s2.get()));
                    voice.Enqueue(MakeStreamSeek(kBlockStart + 247, 200, s3.get()));
                }
            });

            // The intermediate seek's stream and the replaced current stream are each dropped once; s3 never is.
            AM_EXPECT_EQ(static_cast<AmSize>(2), EventsOf(run, eVoiceEventKind::StreamDropped).size());
            AM_EXPECT_EQ(static_cast<AmSize>(1), CountDropped(run, s1.get()));
            AM_EXPECT_EQ(static_cast<AmSize>(1), CountDropped(run, s2.get()));
            AM_EXPECT_EQ(static_cast<AmSize>(0), CountDropped(run, s3.get()));

            const auto swapped = EventsOf(run, eVoiceEventKind::StreamSwapped);
            AM_EXPECT_EQ(static_cast<AmSize>(1), swapped.size());
            if (swapped.size() == 1)
                AM_EXPECT(swapped[0].stream == s3.get());
        }
    };

    AM_REGISTER_TEST(mixer_streaming, voice_direct_reposition_tail_reports_intermediate_streams);
} // namespace SparkyStudios::Audio::Amplitude::Tests
