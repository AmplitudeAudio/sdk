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
    AM_TEST_CASE(DSPTestCase, mixer_voice, voice_release_reports_position)
    {
    public:
        void Run() override
        {
            const AudioBuffer ramp = MakeRamp(48000, 1.0f / 65536.0f);
            auto voice = std::make_unique<Voice>();
            AM_EXPECT(voice->Initialize(MakeVoiceSettings(ramp, 48000, 256)));

            const VoiceRun run = RunVoice(
                *voice, 2048, 256,
                [&](AmUInt64 clock)
                {
                    if (clock == 256)
                        voice->Enqueue(MakeCommand(eVoiceCommandKind::Release, 300, 0.0)); // kStealFade: 480 frames
                });

            const VoiceEvent* fadedOut = FindEvent(run, eVoiceEventKind::FadedOut);
            AM_EXPECT_NOT(fadedOut == nullptr);
            AM_EXPECT(fadedOut->target == eVoiceFadeTarget::Released);
            AM_EXPECT_EQ(780ULL, fadedOut->frame);
            AM_EXPECT_EQ(780ULL, fadedOut->sourcePosition);
            AM_EXPECT(voice->GetPublishedState() == eVoiceState::Finished);
        }
    };

    AM_REGISTER_TEST(mixer_voice, voice_release_reports_position);
} // namespace SparkyStudios::Audio::Amplitude::Tests
