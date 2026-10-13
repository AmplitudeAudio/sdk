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
#include <tuple>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <DSP/ChannelMap.h>

#include "DSPTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(DSPTestCase, dsp_panning, channel_map_itu_coefficients)
    {
    public:
        void Run() override
        {
            constexpr AmReal32 k = 0.70710678f;
            const auto& mono = ChannelLayout::Get(eChannelLayout_Mono);
            const auto& stereo = ChannelLayout::Get(eChannelLayout_Stereo);
            const auto& quad = ChannelLayout::Get(eChannelLayout_Quad);
            const auto& s51 = ChannelLayout::Get(eChannelLayout_Surround_5_1);
            const auto& s71 = ChannelLayout::Get(eChannelLayout_Surround_7_1);

            // Expects exactly the listed (output, input, gain) entries and zeros elsewhere.
            const auto onlyEntries = [this](const ChannelMatrix& m, std::initializer_list<std::tuple<AmUInt16, AmUInt16, AmReal32>> entries)
            {
                for (AmUInt16 o = 0; o < m.outputCount; ++o)
                {
                    for (AmUInt16 i = 0; i < m.inputCount; ++i)
                    {
                        AmReal32 expected = 0.0f;
                        for (const auto& [eo, ei, gain] : entries)
                            if (eo == o && ei == i)
                                expected = gain;

                        AM_EXPECT(std::abs(m.gains[o][i] - expected) < 1e-6f);
                    }
                }
            };

            // Fewer output channels: the ITU fold-down.
            onlyEntries(BuildChannelMap(s51, stereo), { { 0, 0, 1.0f }, { 0, 2, k }, { 0, 4, k }, { 1, 1, 1.0f }, { 1, 2, k }, { 1, 5, k } });
            onlyEntries(BuildChannelMap(quad, stereo), { { 0, 0, 1.0f }, { 0, 2, k }, { 1, 1, 1.0f }, { 1, 3, k } });
            onlyEntries(
                BuildChannelMap(s71, s51),
                { { 0, 0, 1.0f }, { 1, 1, 1.0f }, { 2, 2, 1.0f }, { 3, 3, 1.0f }, { 4, 4, k }, { 4, 6, k }, { 5, 5, k }, { 5, 7, k } });

            // Mono goes to the centre when there is one, otherwise to L and R at -3 dB.
            onlyEntries(BuildChannelMap(mono, s51), { { 2, 0, 1.0f } });
            onlyEntries(BuildChannelMap(mono, stereo), { { 0, 0, k }, { 1, 0, k } });
            onlyEntries(BuildChannelMap(mono, quad), { { 0, 0, k }, { 1, 0, k } });

            // More output channels: each speaker on its nearest output speaker, nothing synthesised.
            onlyEntries(BuildChannelMap(stereo, s51), { { 0, 0, 1.0f }, { 1, 1, 1.0f } });
            onlyEntries(BuildChannelMap(quad, s51), { { 0, 0, 1.0f }, { 1, 1, 1.0f }, { 4, 2, 1.0f }, { 5, 3, 1.0f } });
            onlyEntries(
                BuildChannelMap(s51, s71), { { 0, 0, 1.0f }, { 1, 1, 1.0f }, { 2, 2, 1.0f }, { 3, 3, 1.0f }, { 6, 4, 1.0f }, { 7, 5, 1.0f } });
            onlyEntries(BuildChannelMap(quad, s71), { { 0, 0, 1.0f }, { 1, 1, 1.0f }, { 4, 2, 1.0f }, { 5, 3, 1.0f } });

            // Matching layouts: identity.
            onlyEntries(BuildChannelMap(s71, s71), { { 0, 0, 1.0f }, { 1, 1, 1.0f }, { 2, 2, 1.0f }, { 3, 3, 1.0f }, { 4, 4, 1.0f }, { 5, 5, 1.0f }, { 6, 6, 1.0f }, { 7, 7, 1.0f } });

            // Apply: stereo onto 5.1 leaves the other speakers silent, even over stale content.
            AudioBuffer in(64, 2);
            for (AmUInt64 i = 0; i < 64; ++i)
            {
                in[0][i] = 0.25f + 0.01f * static_cast<AmReal32>(i);
                in[1][i] = -0.5f;
            }

            AudioBuffer out(64, 6);
            for (AmUInt16 c = 0; c < 6; ++c)
                for (AmUInt64 i = 0; i < 64; ++i)
                    out[c][i] = 9.0f;

            ApplyChannelMap(BuildChannelMap(stereo, s51), in, out, 64);
            for (AmUInt64 i = 0; i < 64; ++i)
            {
                AM_EXPECT_EQ(in[0][i], out[0][i]);
                AM_EXPECT_EQ(in[1][i], out[1][i]);
                for (AmUInt16 c = 2; c < 6; ++c)
                    AM_EXPECT_EQ(0.0f, out[c][i]);
            }
        }
    };

    AM_REGISTER_TEST(dsp_panning, channel_map_itu_coefficients);
} // namespace SparkyStudios::Audio::Amplitude::Tests
