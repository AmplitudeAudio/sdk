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
    AM_TEST_CASE(DSPTestCase, mixer_voice, voice_stop_fades_then_finishes)
    {
    public:
        void Run() override
        {
            AudioBuffer dc(48000, 1);
            for (AmUInt64 i = 0; i < 48000; ++i)
                dc[0][i] = 1.0f;

            auto voice = std::make_unique<Voice>();
            AM_EXPECT(voice->Initialize(MakeVoiceSettings(dc, 48000, 256)));

            const VoiceRun run = RunVoice(
                *voice, 2048, 256,
                [&](AmUInt64 clock)
                {
                    if (clock == 256)
                        voice->Enqueue(MakeCommand(eVoiceCommandKind::Stop, 300, 10.0));
                });

            for (AmUInt64 i = 0; i < 300; ++i)
                AM_EXPECT_EQ(1.0f, run.gains[i]);

            // The fade is rounded at both ends (no slope corner), so it is not a straight ramp: it starts flat, passes the
            // middle at one half exactly, only goes down, and ends on zero.
            AM_EXPECT(1.0f - run.gains[300] < 5e-4f);
            AM_EXPECT(std::abs(run.gains[300 + 239] - 0.5f) < 1e-4f);
            for (AmUInt64 k = 1; k < 480; ++k)
                AM_EXPECT(run.gains[300 + k] <= run.gains[300 + k - 1]);
            AM_EXPECT_EQ(0.0f, run.gains[300 + 479]);

            // The source keeps playing under the fade; after it, nothing is rendered.
            AM_EXPECT_EQ(1.0f, run.source[779]);
            AM_EXPECT_EQ(0.0f, run.source[800]);

            const VoiceEvent* fadedOut = FindEvent(run, eVoiceEventKind::FadedOut);
            AM_EXPECT_NOT(fadedOut == nullptr);
            AM_EXPECT(fadedOut->target == eVoiceFadeTarget::Stopped);
            AM_EXPECT_EQ(780ULL, fadedOut->frame);
            const VoiceEvent* finished = FindEvent(run, eVoiceEventKind::Finished);
            AM_EXPECT_NOT(finished == nullptr);
            AM_EXPECT_EQ(780ULL, finished->frame);
            AM_EXPECT(voice->GetPublishedState() == eVoiceState::Finished);
        }
    };

    AM_REGISTER_TEST(mixer_voice, voice_stop_fades_then_finishes);
} // namespace SparkyStudios::Audio::Amplitude::Tests
