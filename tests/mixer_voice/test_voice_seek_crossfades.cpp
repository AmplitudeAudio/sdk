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
    AM_TEST_CASE(DSPTestCase, mixer_voice, voice_seek_crossfades)
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
                    if (clock == 768)
                        voice->Enqueue(MakeCommand(eVoiceCommandKind::Seek, 1000, 0.0, 10000));
                });

            AM_EXPECT_EQ(ramp[0][999], run.source[999]);

            // The seek lands on frame 1000 and the crossfade runs for kSeekCrossfade (8 ms) after it, so the first
            // frame that plays the new position outright is 1000 + 384.
            constexpr AmUInt64 crossfadeEnd = 1384;

            // Inside the crossfade the output lies between the old and the new position.
            const AmReal32 mid = run.source[1120];
            AM_EXPECT(mid > ramp[0][1120] && mid < ramp[0][10120] * 1.5f);

            // After it, the new position plays exactly.
            AM_EXPECT_EQ(ramp[0][10000 + crossfadeEnd - 1000], run.source[crossfadeEnd]);
        }
    };

    AM_REGISTER_TEST(mixer_voice, voice_seek_crossfades);
} // namespace SparkyStudios::Audio::Amplitude::Tests
