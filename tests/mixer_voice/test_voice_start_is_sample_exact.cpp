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
    AM_TEST_CASE(DSPTestCase, mixer_voice, voice_start_is_sample_exact)
    {
    public:
        void Run() override
        {
            AudioBuffer dc(48000, 1);
            for (AmUInt64 i = 0; i < 48000; ++i)
                dc[0][i] = 1.0f;

            VoiceSettings settings = MakeVoiceSettings(dc, 48000, 256);
            settings.startFrame = 1000;
            auto voice = std::make_unique<Voice>();
            AM_EXPECT(voice->Initialize(settings));

            const VoiceRun run = RunVoice(*voice, 2048, 256);
            for (AmUInt64 i = 0; i < 1000; ++i)
                AM_EXPECT_EQ(0.0f, run.source[i]);
            for (AmUInt64 i = 1000; i < 2048; ++i)
                AM_EXPECT_EQ(1.0f, run.source[i]);

            const VoiceEvent* started = FindEvent(run, eVoiceEventKind::Started);
            AM_EXPECT_NOT(started == nullptr);
            AM_EXPECT_EQ(1000ULL, started->frame);
        }
    };

    AM_REGISTER_TEST(mixer_voice, voice_start_is_sample_exact);
} // namespace SparkyStudios::Audio::Amplitude::Tests
