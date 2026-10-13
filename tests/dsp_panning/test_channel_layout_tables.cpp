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

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include "PureUnitTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(PureUnitTestCase, dsp_panning, channel_layout_tables)
    {
    public:
        void Run() override
        {
            const ChannelLayout& s71 = ChannelLayout::Get(eChannelLayout_Surround_7_1);
            constexpr eSpeaker speakers71[] = { eSpeaker_FrontLeft,   eSpeaker_FrontRight, eSpeaker_FrontCenter, eSpeaker_LowFrequency,
                                                eSpeaker_BackLeft,    eSpeaker_BackRight,  eSpeaker_SideLeft,    eSpeaker_SideRight };
            constexpr AmReal32 azimuths71[] = { 30.0f, -30.0f, 0.0f, 0.0f, 150.0f, -150.0f, 90.0f, -90.0f };
            AM_EXPECT_EQ(8, s71.GetChannelCount());
            for (AmUInt16 c = 0; c < 8; ++c)
            {
                AM_EXPECT_EQ(speakers71[c], s71.GetSpeaker(c));
                AM_EXPECT_EQ(azimuths71[c], s71.GetAzimuth(c));
            }
            AM_EXPECT_EQ(3, s71.GetLfeChannel());
            AM_EXPECT(s71.IsLfe(3));
            AM_EXPECT_NOT(s71.IsLfe(4));
            AM_EXPECT_EQ(300.0f, s71.GetNativeWidth());

            const ChannelLayout& s51 = ChannelLayout::Get(eChannelLayout_Surround_5_1);
            constexpr eSpeaker speakers51[] = { eSpeaker_FrontLeft,    eSpeaker_FrontRight, eSpeaker_FrontCenter,
                                                eSpeaker_LowFrequency, eSpeaker_SideLeft,   eSpeaker_SideRight };
            constexpr AmReal32 azimuths51[] = { 30.0f, -30.0f, 0.0f, 0.0f, 110.0f, -110.0f };
            AM_EXPECT_EQ(6, s51.GetChannelCount());
            for (AmUInt16 c = 0; c < 6; ++c)
            {
                AM_EXPECT_EQ(speakers51[c], s51.GetSpeaker(c));
                AM_EXPECT_EQ(azimuths51[c], s51.GetAzimuth(c));
            }
            AM_EXPECT_EQ(220.0f, s51.GetNativeWidth());
            AM_EXPECT_EQ(2, s51.FindSpeaker(eSpeaker_FrontCenter));

            const ChannelLayout& quad = ChannelLayout::Get(eChannelLayout_Quad);
            constexpr AmReal32 azimuthsQuad[] = { 45.0f, -45.0f, 135.0f, -135.0f };
            AM_EXPECT_EQ(4, quad.GetChannelCount());
            for (AmUInt16 c = 0; c < 4; ++c)
                AM_EXPECT_EQ(azimuthsQuad[c], quad.GetAzimuth(c));
            AM_EXPECT_EQ(eSpeaker_BackLeft, quad.GetSpeaker(2));
            AM_EXPECT_EQ(-1, quad.GetLfeChannel());
            AM_EXPECT_EQ(270.0f, quad.GetNativeWidth());

            const ChannelLayout& stereo = ChannelLayout::Get(eChannelLayout_Stereo);
            AM_EXPECT_EQ(30.0f, stereo.GetAzimuth(0));
            AM_EXPECT_EQ(-30.0f, stereo.GetAzimuth(1));
            AM_EXPECT_EQ(60.0f, stereo.GetNativeWidth());
            AM_EXPECT_EQ(-1, stereo.FindSpeaker(eSpeaker_FrontCenter));

            const ChannelLayout& mono = ChannelLayout::Get(eChannelLayout_Mono);
            AM_EXPECT_EQ(eSpeaker_FrontCenter, mono.GetSpeaker(0));
            AM_EXPECT_EQ(0.0f, mono.GetNativeWidth());

            AM_EXPECT(ChannelLayout::FromChannelCount(1) == &mono);
            AM_EXPECT(ChannelLayout::FromChannelCount(2) == &stereo);
            AM_EXPECT(ChannelLayout::FromChannelCount(4) == &quad);
            AM_EXPECT(ChannelLayout::FromChannelCount(6) == &s51);
            AM_EXPECT(ChannelLayout::FromChannelCount(8) == &s71);
            for (const AmUInt16 count : { 0, 3, 5, 7, 9, 16 })
                AM_EXPECT(ChannelLayout::FromChannelCount(count) == nullptr);

            AM_EXPECT(&ChannelLayout::LargestFitting(0) == &stereo);
            AM_EXPECT(&ChannelLayout::LargestFitting(1) == &mono);
            AM_EXPECT(&ChannelLayout::LargestFitting(3) == &stereo);
            AM_EXPECT(&ChannelLayout::LargestFitting(5) == &quad);
            AM_EXPECT(&ChannelLayout::LargestFitting(7) == &s51);
            AM_EXPECT(&ChannelLayout::LargestFitting(12) == &s71);
        }
    };

    AM_REGISTER_TEST(dsp_panning, channel_layout_tables);
} // namespace SparkyStudios::Audio::Amplitude::Tests
