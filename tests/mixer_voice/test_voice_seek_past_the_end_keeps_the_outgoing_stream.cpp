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

#include <cmath>
#include <memory>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Mixer/Voice/Voice.h>

#include "DSPTestCase.h"
#include "TestRegistry.h"
#include "VoiceTestUtils.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(DSPTestCase, mixer_voice, voice_seek_past_the_end_keeps_the_outgoing_stream)
    {
    public:
        void Run() override
        {
            // A seek to within the resampler latency of the end of a non-looping sound: priming the incoming stream reaches
            // the end. That must not finish the voice in the middle of the crossfade (cutting the outgoing stream with no
            // fade): the voice ends once the incoming stream has taken over.
            const AudioBuffer ramp = MakeRamp(44100, 1.0f / 65536.0f);
            const AmUInt64 length = 44100;
            const AmUInt64 seekClock = 1024;
            const AmUInt64 crossfade = 384; // kSeekCrossfade (8 ms) at 48 kHz

            auto voice = std::make_unique<Voice>();
            AM_EXPECT(voice->Initialize(MakeVoiceSettings(ramp, 44100, 256)));
            const VoiceRun run = RunVoice(
                *voice, 8192, 256,
                [&](AmUInt64 clock)
                {
                    if (clock == seekClock)
                        voice->Enqueue(MakeCommand(eVoiceCommandKind::Seek, 0, 0.0, length - 2));
                });

            // The outgoing stream keeps sounding for the whole crossfade (it is a rising ramp, never zero past frame 0).
            for (AmUInt64 i = seekClock + 8; i < seekClock + crossfade - 8; ++i)
                AM_EXPECT_NOT(run.source[i] == 0.0f);

            const VoiceEvent* ended = FindEvent(run, eVoiceEventKind::Ended);
            const VoiceEvent* finished = FindEvent(run, eVoiceEventKind::Finished);
            AM_EXPECT_NOT(ended == nullptr);
            AM_EXPECT_NOT(finished == nullptr);
            if (ended != nullptr)
                AM_EXPECT(ended->frame >= seekClock + crossfade);
            if (finished != nullptr)
                AM_EXPECT(finished->frame >= seekClock + crossfade);
            AM_EXPECT(voice->GetPublishedState() == eVoiceState::Finished);
        }
    };

    AM_REGISTER_TEST(mixer_voice, voice_seek_past_the_end_keeps_the_outgoing_stream);
} // namespace SparkyStudios::Audio::Amplitude::Tests
