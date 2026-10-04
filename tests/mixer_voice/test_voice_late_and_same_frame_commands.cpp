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
    AM_TEST_CASE(DSPTestCase, mixer_voice, voice_late_and_same_frame_commands)
    {
    public:
        void Run() override
        {
            AudioBuffer dc(96000, 1);
            for (AmUInt64 i = 0; i < 96000; ++i)
                dc[0][i] = 1.0f;

            auto voice = std::make_unique<Voice>();
            AM_EXPECT(voice->Initialize(MakeVoiceSettings(dc, 48000, 256)));

            const VoiceRun run = RunVoice(
                *voice, 1024, 256,
                [&](AmUInt64 clock)
                {
                    if (clock != 10000 + 256)
                        return;

                    // Both stamped long before this block, same frame: applied at its first frame, in order.
                    voice->Enqueue(MakeCommand(eVoiceCommandKind::Pause, 5000, 10.0));
                    voice->Enqueue(MakeCommand(eVoiceCommandKind::Resume, 5000, 10.0));
                },
                10000);

            AM_EXPECT_EQ(2U, voice->GetLateCommandCount());
            AM_EXPECT(voice->GetPublishedState() == eVoiceState::Playing);

            // Pause then resume on the same frame never drops below the start of the fade.
            for (AmUInt64 i = 256; i < 1024; ++i)
                AM_EXPECT(run.gains[i] > 0.99f);
        }
    };

    AM_REGISTER_TEST(mixer_voice, voice_late_and_same_frame_commands);
} // namespace SparkyStudios::Audio::Amplitude::Tests
