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
    AM_TEST_CASE(DSPTestCase, mixer_streaming, voice_seek_with_current_stream_reports_nothing)
    {
    public:
        void Run() override
        {
            constexpr AmUInt64 kLength = 48000;
            constexpr AmUInt64 kSeekFrame = 1024;

            auto s1 = MakeSwapStream(kLength, 0, 0);
            Voice voice;
            AM_EXPECT(voice.Initialize(MakeSwapSettings(*s1, kLength)));
            voice.BindStream(s1.get());

            const auto run = RunVoice(voice, 4096, kSwapBlock, [&](AmUInt64 clock) {
                if (clock == kSeekFrame)
                    voice.Enqueue(MakeStreamSeek(kSeekFrame, 3000, s1.get()));
            });

            // The seek carries the stream the voice already reads: no drop, no swap.
            AM_EXPECT(FindEvent(run, eVoiceEventKind::StreamDropped) == nullptr);
            AM_EXPECT(FindEvent(run, eVoiceEventKind::StreamSwapped) == nullptr);
        }
    };

    AM_REGISTER_TEST(mixer_streaming, voice_seek_with_current_stream_reports_nothing);
} // namespace SparkyStudios::Audio::Amplitude::Tests
