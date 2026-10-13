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
    AM_TEST_CASE(DSPTestCase, mixer_voice, voice_seek_while_paused_cancels_pending_crossfade)
    {
    public:
        void Run() override
        {
            const AudioBuffer ramp = MakeRamp(48000, 1.0f / 65536.0f);
            auto voice = std::make_unique<Voice>();
            AM_EXPECT(voice->Initialize(MakeVoiceSettings(ramp, 48000, 256)));

            constexpr AmUInt64 pauseFrame = 1000;   // Pause fade: 5 ms = 240 frames, ends at 1240.
            constexpr AmUInt64 seek1Frame = 1235;   // Lands in the pause fade's last 5 frames.
            constexpr AmUInt64 seek1Position = 20000;
            constexpr AmUInt64 seek2Frame = 2000;   // While Paused: the first seek's crossfade is still pending (frozen).
            constexpr AmUInt64 seek2Position = 8000;
            constexpr AmUInt64 resumeFrame = 2500;

            AmUInt64 midPosition = 0;

            const VoiceRun run = RunVoice(
                *voice, 3072, 256,
                [&](AmUInt64 clock)
                {
                    if (clock == 0)
                    {
                        voice->Enqueue(MakeCommand(eVoiceCommandKind::Pause, pauseFrame, 5.0));
                        voice->Enqueue(MakeCommand(eVoiceCommandKind::Seek, seek1Frame, 0.0, seek1Position));
                        voice->Enqueue(MakeCommand(eVoiceCommandKind::Seek, seek2Frame, 0.0, seek2Position));
                        voice->Enqueue(MakeCommand(eVoiceCommandKind::Resume, resumeFrame, 0.0));
                    }

                    // While frozen (Paused), midway between the first seek's crossfade arming and the second seek:
                    // the published position should already reflect the incoming (first-seek) stream, not the
                    // outgoing one.
                    if (clock == 1536)
                        midPosition = voice->GetPublishedPosition();
                });

            // Untouched before the pause.
            for (AmUInt64 i = 0; i < pauseFrame; ++i)
                AM_EXPECT_EQ(ramp[0][i], run.source[i]);

            // Silent while paused, well before the second seek.
            AM_EXPECT_EQ(0.0f, run.source[1500]);

            // The frozen crossfade's incoming stream (targeting seek1Position) is what gets reported meanwhile.
            AM_EXPECT(midPosition > 15000);

            // The second seek, while Paused, cancels the pending crossfade outright: resuming starts at exactly the
            // second seek's target, never blending in the abandoned first seek.
            AM_EXPECT_EQ(ramp[0][seek2Position], run.source[resumeFrame]);
            AM_EXPECT_EQ(ramp[0][seek2Position + 1], run.source[resumeFrame + 1]);
            AM_EXPECT_EQ(ramp[0][seek2Position + 2], run.source[resumeFrame + 2]);
        }
    };

    AM_REGISTER_TEST(mixer_voice, voice_seek_while_paused_cancels_pending_crossfade);
} // namespace SparkyStudios::Audio::Amplitude::Tests
