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
    AM_TEST_CASE(DSPTestCase, mixer_voice, voice_stop_during_fade_in)
    {
    public:
        void Run() override
        {
            AudioBuffer dc(48000, 1);
            for (AmUInt64 i = 0; i < 48000; ++i)
                dc[0][i] = 1.0f;

            VoiceSettings settings = MakeVoiceSettings(dc, 48000, 256);
            settings.fadeIn = 10.0;
            auto voice = std::make_unique<Voice>();
            AM_EXPECT(voice->Initialize(settings));

            const VoiceRun run = RunVoice(
                *voice, 1024, 256,
                [&](AmUInt64 clock)
                {
                    if (clock == 0)
                        voice->Enqueue(MakeCommand(eVoiceCommandKind::Stop, 240, 10.0));
                });

            // The stop starts from the level the fade-in reached: no jump.
            AM_EXPECT(std::abs(run.gains[240] - run.gains[239]) < 0.01f);
            AM_EXPECT(run.gains[239] > 0.45f && run.gains[239] < 0.55f);
            AM_EXPECT_EQ(0.0f, run.gains[240 + 479]);
            AM_EXPECT(voice->GetPublishedState() == eVoiceState::Finished);
        }
    };

    AM_REGISTER_TEST(mixer_voice, voice_stop_during_fade_in);
} // namespace SparkyStudios::Audio::Amplitude::Tests
