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

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <DSP/SpeakerPanner.h>

#include "PureUnitTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(PureUnitTestCase, dsp_panning, speaker_panner_folds_rear_on_stereo)
    {
    public:
        void Run() override
        {
            const ChannelLayout& stereo = ChannelLayout::Get(eChannelLayout_Stereo);
            const auto same = [&](AmReal32 a, AmReal32 b)
            {
                const SpeakerGains ga = SpeakerPanner::Pan(a, 0.0f, stereo);
                const SpeakerGains gb = SpeakerPanner::Pan(b, 0.0f, stereo);
                return std::abs(ga[0] - gb[0]) < 1e-6f && std::abs(ga[1] - gb[1]) < 1e-6f;
            };

            // Behind folds to the mirror in front, on the same side.
            AM_EXPECT(same(160.0f, 20.0f));
            AM_EXPECT(same(-165.0f, -15.0f));
            AM_EXPECT(same(180.0f, 0.0f));

            // Outside the pair clamps to the nearer speaker.
            const SpeakerGains hardLeft = SpeakerPanner::Pan(90.0f, 0.0f, stereo);
            AM_EXPECT(std::abs(hardLeft[0] - 1.0f) < 1e-6f && std::abs(hardLeft[1]) < 1e-6f);
            const SpeakerGains rearRight = SpeakerPanner::Pan(-120.0f, 0.0f, stereo);
            AM_EXPECT(std::abs(rearRight[1] - 1.0f) < 1e-6f && std::abs(rearRight[0]) < 1e-6f);

            // Mono output: always unity.
            const ChannelLayout& mono = ChannelLayout::Get(eChannelLayout_Mono);
            for (const AmReal32 azimuth : { -170.0f, -30.0f, 0.0f, 75.0f, 180.0f })
                AM_EXPECT_EQ(1.0f, SpeakerPanner::Pan(azimuth, 0.0f, mono)[0]);
        }
    };

    AM_REGISTER_TEST(dsp_panning, speaker_panner_folds_rear_on_stereo);
} // namespace SparkyStudios::Audio::Amplitude::Tests
