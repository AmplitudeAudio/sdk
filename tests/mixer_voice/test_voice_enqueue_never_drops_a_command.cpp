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
    AM_TEST_CASE(DSPTestCase, mixer_voice, voice_enqueue_never_drops_a_command)
    {
    public:
        void Run() override
        {
            AudioBuffer dc(48000, 1);
            for (AmUInt64 i = 0; i < 48000; ++i)
                dc[0][i] = 1.0f;

            auto voice = std::make_unique<Voice>();
            AM_EXPECT(voice->Initialize(MakeVoiceSettings(dc, 48000, 256)));

            // Fill the pending queue with commands stamped far in the future; none of them may fire during this test.
            for (AmSize i = 0; i < Voice::kMaxPendingCommands; ++i)
                voice->Enqueue(MakeCommand(eVoiceCommandKind::Pause, 10000000ULL + i, 10.0));

            // The queue is now full: an ASAP Stop must still be accepted, by evicting the latest-stamped pending
            // command (never by dropping the new one).
            voice->Enqueue(MakeCommand(eVoiceCommandKind::Stop, kVoiceAsap, 100.0));

            const VoiceRun run = RunVoice(*voice, 256, 256);

            AM_EXPECT_EQ(1U, voice->GetEvictedCommandCount());

            // The Stop, sorted ahead of every far-future Pause, is applied at the block's first frame: the voice
            // starts fading out immediately instead of waiting on the (now evicted) latest Pause.
            AM_EXPECT(voice->GetPublishedState() == eVoiceState::FadingOut);
            for (AmUInt64 i = 0; i < 256; ++i)
                AM_EXPECT(run.gains[i] < 1.0f);
        }
    };

    AM_REGISTER_TEST(mixer_voice, voice_enqueue_never_drops_a_command);
} // namespace SparkyStudios::Audio::Amplitude::Tests
