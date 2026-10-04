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
    AM_TEST_CASE(DSPTestCase, mixer_voice, voice_loop_is_seamless)
    {
    public:
        void Run() override
        {
            // Exactly one period of 100 Hz at 48 kHz: a seamless loop renders a perfect sine.
            AudioBuffer period(480, 1);
            for (AmUInt64 i = 0; i < 480; ++i)
                period[0][i] = static_cast<AmReal32>(0.5 * std::sin(2.0 * 3.14159265358979323846 * static_cast<AmReal64>(i) / 480.0));

            auto voice = std::make_unique<Voice>();
            AM_EXPECT(voice->Initialize(MakeVoiceSettings(period, 48000, 1024, true)));

            const VoiceRun run = RunVoice(*voice, 10240, 1024);
            for (AmUInt64 n = 0; n < 10240; ++n)
                AM_EXPECT_EQ(period[0][n % 480], run.source[n]);

            AmUInt32 loops = 0;
            for (const auto& event : run.events)
                if (event.kind == eVoiceEventKind::Looped)
                    loops += event.count;
            AM_EXPECT_EQ(21U, loops); // floor(10240 / 480)
        }
    };

    AM_REGISTER_TEST(mixer_voice, voice_loop_is_seamless);
} // namespace SparkyStudios::Audio::Amplitude::Tests
