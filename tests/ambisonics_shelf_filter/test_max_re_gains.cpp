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

#include <Ambisonics/AmbisonicShelfFilter.h>

#include "DSPTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(DSPTestCase, ambisonics_shelf_filter, max_re_gains_3d)
    {
    public:
        void Run() override
        {
            AmbisonicShelfFilter filter1;
            filter1.Configure(1, true, 512, 48000);
            auto gains1 = filter1.GetMaxReGains();
            AM_EXPECT_EQ(gains1.size(), 2u);
            AM_EXPECT(gains1[0] > 0.0f);
            AM_EXPECT(gains1[1] > 0.0f);
            AM_EXPECT(gains1[1] < gains1[0]);

            AmbisonicShelfFilter filter2;
            filter2.Configure(2, true, 512, 48000);
            auto gains2 = filter2.GetMaxReGains();
            AM_EXPECT_EQ(gains2.size(), 3u);
            AM_EXPECT(gains2[0] > 0.0f);
            AM_EXPECT(gains2[1] > 0.0f);
            AM_EXPECT(gains2[2] > 0.0f);
            AM_EXPECT(gains2[1] < gains2[0]);
            AM_EXPECT(gains2[2] < gains2[1]);

            AmbisonicShelfFilter filter3;
            filter3.Configure(3, true, 512, 48000);
            auto gains3 = filter3.GetMaxReGains();
            AM_EXPECT_EQ(gains3.size(), 4u);
            AM_EXPECT(gains3[0] > 0.0f);
            AM_EXPECT(gains3[1] > 0.0f);
            AM_EXPECT(gains3[2] > 0.0f);
            AM_EXPECT(gains3[3] > 0.0f);
            AM_EXPECT(gains3[1] < gains3[0]);
            AM_EXPECT(gains3[2] < gains3[1]);
            AM_EXPECT(gains3[3] < gains3[2]);
        }
    };

    AM_REGISTER_TEST(ambisonics_shelf_filter, max_re_gains_3d);

    AM_TEST_CASE(DSPTestCase, ambisonics_shelf_filter, max_re_gains_2d)
    {
    public:
        void Run() override
        {
            AmbisonicShelfFilter filter1;
            filter1.Configure(1, false, 512, 48000);
            auto gains1 = filter1.GetMaxReGains();
            AM_EXPECT_EQ(gains1.size(), 2u);

            AmbisonicShelfFilter filter2;
            filter2.Configure(2, false, 512, 48000);
            auto gains2 = filter2.GetMaxReGains();
            AM_EXPECT_EQ(gains2.size(), 3u);

            AmbisonicShelfFilter filter3;
            filter3.Configure(3, false, 512, 48000);
            auto gains3 = filter3.GetMaxReGains();
            AM_EXPECT_EQ(gains3.size(), 4u);

            for (const auto& g : gains1)
                AM_EXPECT(g > 0.0f);
            for (const auto& g : gains2)
                AM_EXPECT(g > 0.0f);
            for (const auto& g : gains3)
                AM_EXPECT(g > 0.0f);
        }
    };

    AM_REGISTER_TEST(ambisonics_shelf_filter, max_re_gains_2d);

    AM_TEST_CASE(DSPTestCase, ambisonics_shelf_filter, max_re_gains_2d_vs_3d_differ)
    {
    public:
        void Run() override
        {
            AmbisonicShelfFilter filter2D;
            filter2D.Configure(2, false, 512, 48000);
            auto gains2D = filter2D.GetMaxReGains();

            AmbisonicShelfFilter filter3D;
            filter3D.Configure(2, true, 512, 48000);
            auto gains3D = filter3D.GetMaxReGains();

            AM_EXPECT_EQ(gains2D.size(), gains3D.size());

            bool hasDifference = false;
            for (size_t i = 0; i < gains2D.size(); ++i)
            {
                if (std::abs(gains2D[i] - gains3D[i]) > 0.01f)
                {
                    hasDifference = true;
                    break;
                }
            }
            AM_EXPECT(hasDifference);
        }
    };

    AM_REGISTER_TEST(ambisonics_shelf_filter, max_re_gains_2d_vs_3d_differ);

    AM_TEST_CASE(DSPTestCase, ambisonics_shelf_filter, set_high_frequency_gains)
    {
    public:
        void Run() override
        {
            AmbisonicShelfFilter filter;
            filter.Configure(2, true, 512, 48000);

            auto originalGains = filter.GetMaxReGains();

            std::vector<AmReal32> customGains = { 1.0f, 0.8f, 0.6f };
            filter.SetHighFrequencyGains(customGains);

            auto gains = filter.GetMaxReGains();
            AM_EXPECT_EQ(gains.size(), 3u);
            ExpectFloatNear(1.0f, gains[0]);
            ExpectFloatNear(0.8f, gains[1]);
            ExpectFloatNear(0.6f, gains[2]);

            std::vector<AmReal32> wrongSizeGains = { 1.0f, 0.8f };
            filter.SetHighFrequencyGains(wrongSizeGains);

            gains = filter.GetMaxReGains();
            for (size_t i = 0; i < gains.size(); ++i)
            {
                ExpectFloatNear(customGains[i], gains[i]);
            }

            std::vector<AmReal32> tooManyGains = { 1.0f, 0.8f, 0.6f, 0.4f };
            filter.SetHighFrequencyGains(tooManyGains);

            gains = filter.GetMaxReGains();
            for (size_t i = 0; i < gains.size(); ++i)
            {
                ExpectFloatNear(customGains[i], gains[i]);
            }
        }
    };

    AM_REGISTER_TEST(ambisonics_shelf_filter, set_high_frequency_gains);
} // namespace SparkyStudios::Audio::Amplitude::Tests
