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
    AM_TEST_CASE(DSPTestCase, mixer_streaming, voice_stream_seek_on_finished_voice_is_dropped)
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
            auto s2 = MakeSwapStream(kLength, 10000, wanted);

            const auto run = RunVoice(voice, 2048, kSwapBlock, [&](AmUInt64 clock) {
                if (clock == 512)
                    voice.Enqueue(MakeCommand(eVoiceCommandKind::Stop, 512, 0.0));
                else if (clock == 1536)
                    voice.Enqueue(MakeStreamSeek(1536, 10000, s2.get()));
            });

            AM_EXPECT(voice.GetState() == eVoiceState::Finished);

            const auto dropped = EventsOf(run, eVoiceEventKind::StreamDropped);
            AM_EXPECT_EQ(static_cast<AmSize>(1), dropped.size());
            if (dropped.size() == 1)
                AM_EXPECT(dropped[0].stream == s2.get());

            AM_EXPECT(FindEvent(run, eVoiceEventKind::StreamSwapped) == nullptr);
        }
    };

    AM_REGISTER_TEST(mixer_streaming, voice_stream_seek_on_finished_voice_is_dropped);
} // namespace SparkyStudios::Audio::Amplitude::Tests
