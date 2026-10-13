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
    // A switch container replaces its sounds per voice: the outgoing voice gets a Stop with the fade-out duration at the
    // frame the incoming voice starts with the same fade-in. Both fades use the same envelope, so the two gains always add
    // up to one: the replacement neither dips nor swells.
    AM_TEST_CASE(DSPTestCase, mixer_voice, voice_switch_crossfade_sums_to_unity)
    {
    public:
        void Run() override
        {
            AudioBuffer dc(48000, 1);
            for (AmUInt64 i = 0; i < 48000; ++i)
                dc[0][i] = 1.0f;

            constexpr AmUInt64 switchFrame = 1024;
            constexpr AmTime fade = 20.0; // 960 frames at 48 kHz

            auto outgoing = std::make_unique<Voice>();
            AM_EXPECT(outgoing->Initialize(MakeVoiceSettings(dc, 48000, 256)));

            VoiceSettings incomingSettings = MakeVoiceSettings(dc, 48000, 256);
            incomingSettings.startFrame = switchFrame;
            incomingSettings.fadeIn = fade;
            incomingSettings.layer = 2;
            incomingSettings.id = 2;
            auto incoming = std::make_unique<Voice>();
            AM_EXPECT(incoming->Initialize(incomingSettings));

            const VoiceRun out = RunVoice(
                *outgoing, 3072, 256,
                [&](AmUInt64 clock)
                {
                    if (clock == 768)
                        outgoing->Enqueue(MakeCommand(eVoiceCommandKind::Stop, switchFrame, fade));
                });
            const VoiceRun in = RunVoice(*incoming, 3072, 256);

            for (AmUInt64 i = 0; i < switchFrame; ++i)
            {
                AM_EXPECT_EQ(1.0f, out.gains[i]);
                AM_EXPECT_EQ(0.0f, in.gains[i]);
            }

            // Across the fades the two voices add up to one, frame by frame, and the incoming one never leads the
            // outgoing one's loss.
            for (AmUInt64 i = switchFrame; i < switchFrame + 960; ++i)
                AM_EXPECT(std::abs(out.gains[i] + in.gains[i] - 1.0f) < 1e-5f);

            AM_EXPECT_EQ(0.0f, out.gains[switchFrame + 960]);
            AM_EXPECT_EQ(1.0f, in.gains[switchFrame + 960]);
        }
    };

    AM_REGISTER_TEST(mixer_voice, voice_switch_crossfade_sums_to_unity);
} // namespace SparkyStudios::Audio::Amplitude::Tests
