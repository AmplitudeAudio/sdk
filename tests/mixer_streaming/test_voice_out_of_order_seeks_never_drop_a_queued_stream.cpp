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
    AM_TEST_CASE(DSPTestCase, mixer_streaming, voice_out_of_order_seeks_never_drop_a_queued_stream)
    {
    public:
        void Run() override
        {
            constexpr AmUInt64 kLength = 48000;
            constexpr AmUInt64 kIssue = 1024;
            constexpr AmUInt64 kLater = 2048;

            auto s1 = MakeSwapStream(kLength, 0, 0);
            Voice voice;
            AM_EXPECT(voice.Initialize(MakeSwapSettings(*s1, kLength)));
            voice.BindStream(s1.get());

            const AmUInt64 wanted = voice.GetPrerollFrames();
            auto s2 = MakeSwapStream(kLength, 20000, wanted);
            auto s3 = MakeSwapStream(kLength, 30000, wanted);

            // s2 is issued first but stamped later: s3 (ASAP) applies first while s2 is still queued.
            const auto run = RunVoice(voice, 4096, kSwapBlock, [&](AmUInt64 clock) {
                if (clock == kIssue)
                {
                    voice.Enqueue(MakeStreamSeek(kLater, 20000, s2.get()));
                    voice.Enqueue(MakeStreamSeek(kVoiceAsap, 30000, s3.get()));
                }
            });

            // s2 is never reported dropped: it is adopted at its frame, and is current at the end.
            AM_EXPECT_EQ(static_cast<AmSize>(0), CountDropped(run, s2.get()));
            AM_EXPECT_EQ(static_cast<AmSize>(1), CountSwapped(run, s2.get()));

            // s3 and s1 are each dropped once, and s3 only after it was replaced by s2.
            AM_EXPECT_EQ(static_cast<AmSize>(1), CountDropped(run, s1.get()));
            AM_EXPECT_EQ(static_cast<AmSize>(1), CountDropped(run, s3.get()));
            AM_EXPECT_EQ(static_cast<AmSize>(1), CountSwapped(run, s3.get()));

            const auto indexOf = [&](eVoiceEventKind kind, const StreamSource* stream) {
                for (AmSize i = 0; i < run.events.size(); ++i)
                    if (run.events[i].kind == kind && run.events[i].stream == stream)
                        return i;
                return run.events.size();
            };

            const AmSize swapS3 = indexOf(eVoiceEventKind::StreamSwapped, s3.get());
            const AmSize swapS2 = indexOf(eVoiceEventKind::StreamSwapped, s2.get());
            const AmSize dropS3 = indexOf(eVoiceEventKind::StreamDropped, s3.get());
            AM_EXPECT(swapS3 < swapS2);
            AM_EXPECT(swapS2 < dropS3);
        }
    };

    AM_REGISTER_TEST(mixer_streaming, voice_out_of_order_seeks_never_drop_a_queued_stream);
} // namespace SparkyStudios::Audio::Amplitude::Tests
