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
    AM_TEST_CASE(DSPTestCase, mixer_voice, voice_pause_freezes_and_resume_continues)
    {
    public:
        void Run() override
        {
            const AudioBuffer ramp = MakeRamp(48000, 1.0f / 65536.0f);
            auto voice = std::make_unique<Voice>();
            AM_EXPECT(voice->Initialize(MakeVoiceSettings(ramp, 48000, 256)));

            const VoiceRun run = RunVoice(
                *voice, 4096, 256,
                [&](AmUInt64 clock)
                {
                    if (clock == 256)
                        voice->Enqueue(MakeCommand(eVoiceCommandKind::Pause, 500, 0.0)); // 240-frame de-click
                    if (clock == 1792)
                        voice->Enqueue(MakeCommand(eVoiceCommandKind::Resume, 2000, 0.0));
                });

            // The source played through the pause fade (frames 500..739), then froze.
            AM_EXPECT_EQ(ramp[0][739], run.source[739]);
            AM_EXPECT_EQ(0.0f, run.source[1000]);

            // Resume continues from the exact next source frame.
            AM_EXPECT_EQ(ramp[0][740], run.source[2000]);
            AM_EXPECT_EQ(ramp[0][741], run.source[2001]);

            const VoiceEvent* fadedOut = FindEvent(run, eVoiceEventKind::FadedOut);
            AM_EXPECT_NOT(fadedOut == nullptr);
            AM_EXPECT(fadedOut->target == eVoiceFadeTarget::Paused);
            AM_EXPECT_EQ(740ULL, fadedOut->frame);
            AM_EXPECT_EQ(740ULL, fadedOut->sourcePosition);
            AM_EXPECT(voice->GetPublishedState() == eVoiceState::Playing);
        }
    };

    AM_REGISTER_TEST(mixer_voice, voice_pause_freezes_and_resume_continues);
} // namespace SparkyStudios::Audio::Amplitude::Tests
