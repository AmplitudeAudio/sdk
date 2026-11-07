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
    AM_TEST_CASE(DSPTestCase, mixer_streaming, voice_paused_seek_cancels_crossfade_without_spurious_swap)
    {
    public:
        void Run() override
        {
            constexpr AmUInt64 kLength = 48000;
            constexpr AmUInt64 kPauseFrame = 1024;
            constexpr AmUInt64 kSeekFrame = kPauseFrame + 230; // Arms 10 frames before the pause fade ends.
            constexpr AmUInt64 kCurrentSeek = 2048;
            constexpr AmUInt64 kNewSeek = 3072;

            auto s1 = MakeSwapStream(kLength, 0, 0);
            Voice voice;
            AM_EXPECT(voice.Initialize(MakeSwapSettings(*s1, kLength)));
            voice.BindStream(s1.get());

            const AmUInt64 wanted = voice.GetPrerollFrames();
            auto s2 = MakeSwapStream(kLength, 20000, wanted);
            auto s3 = MakeSwapStream(kLength, 30000, wanted);

            const auto run = RunVoice(voice, 4096, kSwapBlock, [&](AmUInt64 clock) {
                if (clock == kPauseFrame)
                {
                    voice.Enqueue(MakeCommand(eVoiceCommandKind::Pause, kPauseFrame, 0.0));
                    voice.Enqueue(MakeStreamSeek(kSeekFrame, 20000, s2.get()));
                }
                else if (clock == kCurrentSeek)
                {
                    // Re-targets the voice's own stream: the current stream does not change.
                    voice.Enqueue(MakeStreamSeek(kCurrentSeek, 500, s1.get()));
                }
                else if (clock == kNewSeek)
                {
                    voice.Enqueue(MakeStreamSeek(kNewSeek, 30000, s3.get()));
                }
            });

            AM_EXPECT(voice.GetState() == eVoiceState::Paused);

            // The incoming stream lost the cancelled crossfade: reported dropped, never swapped to.
            // Then the paused voice moves to s3: s2 (never read) and s1 (replaced) are dropped once each, s3 never.
            AM_EXPECT_EQ(static_cast<AmSize>(2), EventsOf(run, eVoiceEventKind::StreamDropped).size());
            AM_EXPECT_EQ(static_cast<AmSize>(1), CountDropped(run, s1.get()));
            AM_EXPECT_EQ(static_cast<AmSize>(1), CountDropped(run, s2.get()));
            AM_EXPECT_EQ(static_cast<AmSize>(0), CountDropped(run, s3.get()));

            // Only the seek that binds a new stream swaps; the one that kept the current stream does not.
            const auto swapped = EventsOf(run, eVoiceEventKind::StreamSwapped);
            AM_EXPECT_EQ(static_cast<AmSize>(1), swapped.size());
            if (swapped.size() == 1)
                AM_EXPECT(swapped[0].stream == s3.get());
        }
    };

    AM_REGISTER_TEST(mixer_streaming, voice_paused_seek_cancels_crossfade_without_spurious_swap);
} // namespace SparkyStudios::Audio::Amplitude::Tests
