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

#include <algorithm>
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
    AM_TEST_CASE(DSPTestCase, mixer_voice, voice_seek_crossfade_has_no_corners)
    {
    public:
        void Run() override
        {
            // Seeking between two levels crossfades from one to the other. A crossfade with a linear time axis starts
            // (incoming) or ends (outgoing) with a slope corner, an audible tick; the raised-cosine time warp has none.
            for (const bool rising : { true, false })
            {
                AudioBuffer levels(20000, 1);
                for (AmUInt64 i = 0; i < 20000; ++i)
                    levels[0][i] = (i < 10000) == rising ? 0.0f : 1.0f;

                auto voice = std::make_unique<Voice>();
                AM_EXPECT(voice->Initialize(MakeVoiceSettings(levels, 48000, 256)));

                const VoiceRun run = RunVoice(
                    *voice, 3072, 256,
                    [&](AmUInt64 clock)
                    {
                        if (clock == 1024)
                            voice->Enqueue(MakeCommand(eVoiceCommandKind::Seek, 1024, 0.0, 15000));
                    });

                // The crossfade really ran: the output ends on the other level.
                AM_EXPECT(std::abs(run.source[2500] - (rising ? 1.0f : 0.0f)) < 1e-3f);

                // A corner is a jump of the step size: about 6.5e-3 per frame at the ends of a 5 ms linear crossfade.
                AmReal32 worst = 0.0f;
                for (AmUInt64 i = 900; i < 1800; ++i)
                    worst = std::max(worst, std::abs((run.source[i + 1] - run.source[i]) - (run.source[i] - run.source[i - 1])));

                AM_EXPECT(worst < 5e-4f);
            }
        }
    };

    AM_REGISTER_TEST(mixer_voice, voice_seek_crossfade_has_no_corners);
} // namespace SparkyStudios::Audio::Amplitude::Tests
