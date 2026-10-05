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
    namespace
    {
        constexpr AmReal64 kHalfPi = 1.57079632679489661923;
        constexpr AmUInt64 kCrossfadeFrames = 384; // kSeekCrossfade (8 ms) at 48 kHz.

        AmReal32 MaxDelta(const std::vector<AmReal32>& source, AmUInt64 begin, AmUInt64 end)
        {
            AmReal32 worst = 0.0f;
            for (AmUInt64 i = begin + 1; i < end; ++i)
                worst = std::max(worst, std::abs(source[i] - source[i - 1]));
            return worst;
        }
    } // namespace

    AM_TEST_CASE(DSPTestCase, mixer_voice, voice_seek_during_crossfade_stays_bounded)
    {
    public:
        void Run() override
        {
            const AmReal32 step = 1.0f / 65536.0f;
            const AudioBuffer ramp = MakeRamp(48000, step);

            // Two seeks stamped on the same frame: the first is superseded before any audio ever renders for it, so
            // the output must be exactly as if only the second seek had happened, with no discontinuity at all.
            {
                auto voice = std::make_unique<Voice>();
                AM_EXPECT(voice->Initialize(MakeVoiceSettings(ramp, 48000, 256)));

                constexpr AmUInt64 posA = 30000;
                constexpr AmUInt64 posB = 100;

                const VoiceRun run = RunVoice(
                    *voice, 2048, 256,
                    [&](AmUInt64 clock)
                    {
                        if (clock == 256)
                        {
                            voice->Enqueue(MakeCommand(eVoiceCommandKind::Seek, 300, 0.0, posA));
                            voice->Enqueue(MakeCommand(eVoiceCommandKind::Seek, 300, 0.0, posB));
                        }
                    });

                for (AmUInt64 i = 0; i < 300; ++i)
                    AM_EXPECT_EQ(ramp[0][i], run.source[i]);

                // posA's much larger amplitude (ramp[30000] is ~300x ramp[300]) never leaks in: the blend stays small.
                AM_EXPECT(MaxDelta(run.source, 299, 300 + kCrossfadeFrames) < 0.01f);

                AM_EXPECT_EQ(ramp[0][posB + kCrossfadeFrames], run.source[300 + kCrossfadeFrames]);
                AM_EXPECT_EQ(ramp[0][posB + kCrossfadeFrames + 1], run.source[300 + kCrossfadeFrames + 1]);
            }

            // Two seeks 10 frames apart: the second lands while the first crossfade's incoming stream (posA) is only
            // 10/240 of the way in, far from dominant, so its abandoned contribution must stay weighted down instead
            // of being swapped in whole.
            {
                auto voice = std::make_unique<Voice>();
                AM_EXPECT(voice->Initialize(MakeVoiceSettings(ramp, 48000, 256)));

                constexpr AmUInt64 posA = 30000;
                constexpr AmUInt64 posB = 100;
                constexpr AmUInt64 seek1Frame = 500;
                constexpr AmUInt64 seek2Frame = 510;
                constexpr AmUInt64 gap = seek2Frame - seek1Frame; // 10: real crossfade frames posA got before being abandoned.

                const VoiceRun run = RunVoice(
                    *voice, 2048, 256,
                    [&](AmUInt64 clock)
                    {
                        if (clock == 256)
                        {
                            voice->Enqueue(MakeCommand(eVoiceCommandKind::Seek, seek1Frame, 0.0, posA));
                            voice->Enqueue(MakeCommand(eVoiceCommandKind::Seek, seek2Frame, 0.0, posB));
                        }
                    });

                for (AmUInt64 i = 0; i < seek1Frame; ++i)
                    AM_EXPECT_EQ(ramp[0][i], run.source[i]);

                // Concrete bound derived from the ramp: the largest possible single-frame jump is the equal-power
                // weight posA's last rendered frame carried (its contribution disappears at the reseek) times the
                // largest value it could have contributed, plus slack for the ordinary small blend motion elsewhere.
                const AmReal64 seamWeight =
                    std::sin((static_cast<AmReal64>(gap - 1) + 0.5) / static_cast<AmReal64>(kCrossfadeFrames) * kHalfPi);
                const AmReal32 abandonedPeak = ramp[0][posA + gap - 1];
                const AmReal32 bound = static_cast<AmReal32>(seamWeight * abandonedPeak) + 0.01f;

                AM_EXPECT(MaxDelta(run.source, seek1Frame - 1, seek2Frame + kCrossfadeFrames + 10) < bound);

                // The bound is meaningfully tighter than a hard cut, which would have jumped by close to the full
                // abandoned amplitude instead of its small equal-power weight.
                AM_EXPECT(bound < 0.5f * abandonedPeak);

                // After the second crossfade completes, the source plays posB's target exactly; posA never surfaces.
                const AmUInt64 settled = seek2Frame + kCrossfadeFrames;
                AM_EXPECT_EQ(ramp[0][posB + kCrossfadeFrames], run.source[settled]);
                AM_EXPECT_EQ(ramp[0][posB + kCrossfadeFrames + 1], run.source[settled + 1]);
            }
        }
    };

    AM_REGISTER_TEST(mixer_voice, voice_seek_during_crossfade_stays_bounded);
} // namespace SparkyStudios::Audio::Amplitude::Tests
