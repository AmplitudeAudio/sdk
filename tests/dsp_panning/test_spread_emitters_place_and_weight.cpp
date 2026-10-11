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

#include <DSP/SpreadEmitters.h>

#include "PureUnitTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(PureUnitTestCase, dsp_panning, spread_emitters_place_and_weight)
    {
    public:
        void Run() override
        {
            constexpr AmReal32 k = 0.70710678f;
            const auto isClose = [](AmReal32 a, AmReal32 b)
            {
                return std::abs(a - b) < 1e-4f;
            };
            const auto expect = [&](const SpreadEmitterSet& set, std::initializer_list<std::pair<AmReal32, AmReal32>> azimuthWeight)
            {
                AM_EXPECT_EQ(static_cast<AmUInt16>(azimuthWeight.size()), set.count);
                AmUInt16 e = 0;
                for (const auto& [azimuth, weight] : azimuthWeight)
                {
                    AM_EXPECT(isClose(set.emitters[e].azimuth, azimuth));
                    AM_EXPECT(isClose(set.emitters[e].weight, weight));
                    ++e;
                }
            };

            const auto& stereo = ChannelLayout::Get(eChannelLayout_Stereo);
            expect(SpreadEmitters::Compute(stereo, 0.0f, 0.0f, 0.0f), { { 0.0f, k }, { 0.0f, k } });
            expect(SpreadEmitters::Compute(stereo, 0.0f, 0.0f, 30.0f), { { 15.0f, k + (1.0f - k) * 0.5f }, { -15.0f, k + (1.0f - k) * 0.5f } });
            expect(SpreadEmitters::Compute(stereo, 0.0f, 0.0f, 60.0f), { { 30.0f, 1.0f }, { -30.0f, 1.0f } });
            // Halfway from native (60) to 360: halfway from +-30 to +-90.
            expect(SpreadEmitters::Compute(stereo, 0.0f, 0.0f, 210.0f), { { 60.0f, 1.0f }, { -60.0f, 1.0f } });
            expect(SpreadEmitters::Compute(stereo, 0.0f, 0.0f, 360.0f), { { 90.0f, 1.0f }, { -90.0f, 1.0f } });
            // The image follows the centre direction.
            expect(SpreadEmitters::Compute(stereo, 90.0f, 0.0f, 60.0f), { { 120.0f, 1.0f }, { 60.0f, 1.0f } });
            expect(SpreadEmitters::Compute(stereo, 170.0f, 0.0f, 60.0f), { { -160.0f, 1.0f }, { 140.0f, 1.0f } });

            // Evenly spaced at 360, left/right order kept.
            const auto& quad = ChannelLayout::Get(eChannelLayout_Quad);
            expect(SpreadEmitters::Compute(quad, 0.0f, 0.0f, 360.0f), { { 45.0f, 1.0f }, { -45.0f, 1.0f }, { 135.0f, 1.0f }, { -135.0f, 1.0f } });

            // 5.1: the LFE is not an emitter; 5 emitters 72 degrees apart.
            const auto& s51 = ChannelLayout::Get(eChannelLayout_Surround_5_1);
            const SpreadEmitterSet full51 = SpreadEmitters::Compute(s51, 0.0f, 0.0f, 360.0f);
            expect(full51, { { 72.0f, 1.0f }, { -72.0f, 1.0f }, { 0.0f, 1.0f }, { 144.0f, 1.0f }, { -144.0f, 1.0f } });
            AM_EXPECT_EQ(4, full51.emitters[3].channel);

            // Spread 0: every emitter at the centre, weighted by the ITU fold-down to mono.
            const SpreadEmitterSet zero51 = SpreadEmitters::Compute(s51, 25.0f, 10.0f, 0.0f);
            const ChannelMatrix fold = ChannelLayout::GetFoldDownMatrix(s51, ChannelLayout::Get(eChannelLayout_Mono));
            for (AmUInt16 e = 0; e < zero51.count; ++e)
            {
                AM_EXPECT(isClose(zero51.emitters[e].azimuth, 25.0f));
                AM_EXPECT(isClose(zero51.emitters[e].elevation, 10.0f));
                AM_EXPECT(isClose(zero51.emitters[e].weight, fold.gains[0][zero51.emitters[e].channel]));
            }

            // Mono: one emitter at the centre, unit weight, whatever the spread.
            const auto& mono = ChannelLayout::Get(eChannelLayout_Mono);
            for (const AmReal32 spread : { 0.0f, 90.0f, 360.0f })
                expect(SpreadEmitters::Compute(mono, -40.0f, 0.0f, spread), { { -40.0f, 1.0f } });
        }
    };

    AM_REGISTER_TEST(dsp_panning, spread_emitters_place_and_weight);
} // namespace SparkyStudios::Audio::Amplitude::Tests
