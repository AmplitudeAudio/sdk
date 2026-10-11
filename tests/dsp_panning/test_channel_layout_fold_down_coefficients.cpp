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

#include "PureUnitTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(PureUnitTestCase, dsp_panning, channel_layout_fold_down_coefficients)
    {
    public:
        void Run() override
        {
            constexpr AmReal32 k = 0.70710678f;
            const auto isClose = [](AmReal32 a, AmReal32 b)
            {
                return std::abs(a - b) < 1e-6f;
            };
            const auto rowIs = [&](const ChannelMatrix& m, AmUInt16 row, std::initializer_list<AmReal32> expected)
            {
                AmUInt16 i = 0;
                for (const AmReal32 value : expected)
                    AM_EXPECT(isClose(m.gains[row][i++], value));
            };

            const auto& mono = ChannelLayout::Get(eChannelLayout_Mono);
            const auto& stereo = ChannelLayout::Get(eChannelLayout_Stereo);
            const auto& quad = ChannelLayout::Get(eChannelLayout_Quad);
            const auto& s51 = ChannelLayout::Get(eChannelLayout_Surround_5_1);
            const auto& s71 = ChannelLayout::Get(eChannelLayout_Surround_7_1);

            // ITU-R BS.775 5.1 -> stereo: centre and surround at -3 dB, LFE dropped.
            const ChannelMatrix s51ToStereo = ChannelLayout::GetFoldDownMatrix(s51, stereo);
            AM_EXPECT_EQ(6, s51ToStereo.inputCount);
            AM_EXPECT_EQ(2, s51ToStereo.outputCount);
            rowIs(s51ToStereo, 0, { 1.0f, 0.0f, k, 0.0f, k, 0.0f });
            rowIs(s51ToStereo, 1, { 0.0f, 1.0f, k, 0.0f, 0.0f, k });

            // 7.1 -> 5.1: each surround takes its side and back speakers at -3 dB.
            const ChannelMatrix s71To51 = ChannelLayout::GetFoldDownMatrix(s71, s51);
            rowIs(s71To51, 0, { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f });
            rowIs(s71To51, 3, { 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f });
            rowIs(s71To51, 4, { 0.0f, 0.0f, 0.0f, 0.0f, k, 0.0f, k, 0.0f });
            rowIs(s71To51, 5, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, k, 0.0f, k });

            const ChannelMatrix quadToStereo = ChannelLayout::GetFoldDownMatrix(quad, stereo);
            rowIs(quadToStereo, 0, { 1.0f, 0.0f, k, 0.0f });
            rowIs(quadToStereo, 1, { 0.0f, 1.0f, 0.0f, k });

            rowIs(ChannelLayout::GetFoldDownMatrix(stereo, mono), 0, { k, k });

            // 5.1 -> mono through stereo is the ITU mono row: 0.707 L + 0.707 R + C + 0.5 Ls + 0.5 Rs.
            rowIs(ChannelLayout::GetFoldDownMatrix(s51, mono), 0, { k, k, 1.0f, 0.0f, 0.5f, 0.5f });

            // 5.1 -> quad: centre into the front pair, surrounds to the rears.
            const ChannelMatrix s51ToQuad = ChannelLayout::GetFoldDownMatrix(s51, quad);
            rowIs(s51ToQuad, 0, { 1.0f, 0.0f, k, 0.0f, 0.0f, 0.0f });
            rowIs(s51ToQuad, 2, { 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f });

            const ChannelMatrix identity = ChannelLayout::GetFoldDownMatrix(s51, s51);
            for (AmUInt16 o = 0; o < 6; ++o)
                for (AmUInt16 i = 0; i < 6; ++i)
                    AM_EXPECT_EQ(o == i ? 1.0f : 0.0f, identity.gains[o][i]);

            // Folding never goes up in channel count.
            AM_EXPECT_EQ(0, ChannelLayout::GetFoldDownMatrix(stereo, s51).inputCount);
        }
    };

    AM_REGISTER_TEST(dsp_panning, channel_layout_fold_down_coefficients);
} // namespace SparkyStudios::Audio::Amplitude::Tests
