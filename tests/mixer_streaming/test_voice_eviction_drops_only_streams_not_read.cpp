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
    AM_TEST_CASE(DSPTestCase, mixer_streaming, voice_eviction_drops_only_streams_not_read)
    {
    public:
        void Run() override
        {
            constexpr AmUInt64 kLength = 48000;

            auto s1 = MakeSwapStream(kLength, 0, 0);
            Voice voice;
            AM_EXPECT(voice.Initialize(MakeSwapSettings(*s1, kLength)));
            voice.BindStream(s1.get());

            const AmUInt64 wanted = voice.GetPrerollFrames();
            auto s2 = MakeSwapStream(kLength, 20000, wanted);

            // Fill the queue: two far-future stream seeks (the latest-stamped, so evicted first) and ASAP resumes.
            voice.Enqueue(MakeStreamSeek(200000, 500, s1.get()));
            voice.Enqueue(MakeStreamSeek(100000, 20000, s2.get()));
            for (AmSize i = 0; i + 2 < Voice::kMaxPendingCommands; ++i)
                voice.Enqueue(MakeCommand(eVoiceCommandKind::Resume, kVoiceAsap));

            AM_EXPECT_EQ(static_cast<AmUInt32>(0), voice.GetEvictedCommandCount());

            // Evicts the seek carrying the current stream: nothing to report.
            voice.Enqueue(MakeCommand(eVoiceCommandKind::Resume, kVoiceAsap));

            // Evicts the seek carrying s2: reported once.
            voice.Enqueue(MakeCommand(eVoiceCommandKind::Resume, kVoiceAsap));

            const auto run = RunVoice(voice, 1024, kSwapBlock);

            AM_EXPECT_EQ(static_cast<AmUInt32>(2), voice.GetEvictedCommandCount());
            AM_EXPECT_EQ(static_cast<AmSize>(1), CountDropped(run, s2.get()));
            AM_EXPECT_EQ(static_cast<AmSize>(0), CountDropped(run, s1.get()));
            AM_EXPECT_EQ(static_cast<AmSize>(1), EventsOf(run, eVoiceEventKind::StreamDropped).size());
            AM_EXPECT(FindEvent(run, eVoiceEventKind::StreamSwapped) == nullptr);
        }
    };

    AM_REGISTER_TEST(mixer_streaming, voice_eviction_drops_only_streams_not_read);
} // namespace SparkyStudios::Audio::Amplitude::Tests
