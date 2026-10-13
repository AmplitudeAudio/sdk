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
    AM_TEST_CASE(DSPTestCase, mixer_voice, voice_end_of_sound_events)
    {
    public:
        void Run() override
        {
            const AudioBuffer ramp = MakeRamp(1000, 1.0f / 1024.0f);
            auto voice = std::make_unique<Voice>();
            AM_EXPECT(voice->Initialize(MakeVoiceSettings(ramp, 48000, 256)));

            const VoiceRun run = RunVoice(*voice, 2048, 256);

            // Every source frame is played, including the last one.
            for (AmUInt64 i = 0; i < 1000; ++i)
                AM_EXPECT_EQ(ramp[0][i], run.source[i]);
            for (AmUInt64 i = 1000; i < 2048; ++i)
                AM_EXPECT_EQ(0.0f, run.source[i]);

            const VoiceEvent* ended = FindEvent(run, eVoiceEventKind::Ended);
            AM_EXPECT_NOT(ended == nullptr);
            AM_EXPECT_EQ(1000ULL, ended->frame);
            AM_EXPECT_NOT(FindEvent(run, eVoiceEventKind::Finished) == nullptr);
            AM_EXPECT(voice->GetPublishedState() == eVoiceState::Finished);
        }
    };

    AM_REGISTER_TEST(mixer_voice, voice_end_of_sound_events);
} // namespace SparkyStudios::Audio::Amplitude::Tests
