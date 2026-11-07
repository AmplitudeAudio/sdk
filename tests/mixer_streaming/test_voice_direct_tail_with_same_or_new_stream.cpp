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
    AM_TEST_CASE(DSPTestCase, mixer_streaming, voice_direct_tail_with_same_or_new_stream)
    {
    public:
        void Run() override
        {
            // The source ends inside the block, before the stream seeks stamped after the end: they reposition directly.
            constexpr AmUInt64 kLength = 1000;
            constexpr AmUInt64 kBlockStart = 768;

            for (const bool toNewStream : { false, true })
            {
                auto s1 = MakeSwapStream(kLength, 0, 0);
                Voice voice;
                AM_EXPECT(voice.Initialize(MakeSwapSettings(*s1, kLength)));
                voice.BindStream(s1.get());

                const AmUInt64 wanted = voice.GetPrerollFrames();
                auto s3 = MakeSwapStream(kLength, 200, wanted);
                StreamSource* last = toNewStream ? s3.get() : s1.get();

                const auto run = RunVoice(voice, 1280, kSwapBlock, [&](AmUInt64 clock) {
                    if (clock == kBlockStart)
                    {
                        voice.Enqueue(MakeStreamSeek(kBlockStart + 242, 100, s1.get()));
                        voice.Enqueue(MakeStreamSeek(kBlockStart + 247, 200, last));
                    }
                });

                if (toNewStream)
                {
                    // [s1, s3]: s1 is replaced and reported once, not twice; s3 is adopted.
                    AM_EXPECT_EQ(static_cast<AmSize>(1), CountDropped(run, s1.get()));
                    AM_EXPECT_EQ(static_cast<AmSize>(0), CountDropped(run, s3.get()));
                    AM_EXPECT_EQ(static_cast<AmSize>(1), CountSwapped(run, s3.get()));
                }
                else
                {
                    // [s1, s1]: s1 stays current: no drop, no swap.
                    AM_EXPECT(FindEvent(run, eVoiceEventKind::StreamDropped) == nullptr);
                    AM_EXPECT(FindEvent(run, eVoiceEventKind::StreamSwapped) == nullptr);
                }
            }
        }
    };

    AM_REGISTER_TEST(mixer_streaming, voice_direct_tail_with_same_or_new_stream);
} // namespace SparkyStudios::Audio::Amplitude::Tests
