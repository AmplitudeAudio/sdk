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
    AM_TEST_CASE(DSPTestCase, ambisonics_shelf_filter, configure_valid_parameters)
    {
    public:
        void Run() override
        {
            AmbisonicShelfFilter filter;

            AM_EXPECT(filter.Configure(1, true, 512, 48000));
            AM_EXPECT_EQ(filter.GetOrder(), 1u);
            AM_EXPECT(filter.Is3D());

            AM_EXPECT(filter.Configure(2, false, 1024, 44100));
            AM_EXPECT_EQ(filter.GetOrder(), 2u);
            AM_EXPECT_NOT(filter.Is3D());

            AM_EXPECT(filter.Configure(3, true, 2048, 96000));
            AM_EXPECT_EQ(filter.GetOrder(), 3u);
            AM_EXPECT(filter.Is3D());
        }
    };

    AM_REGISTER_TEST(ambisonics_shelf_filter, configure_valid_parameters);

    AM_TEST_CASE(DSPTestCase, ambisonics_shelf_filter, configure_fails_invalid_order)
    {
    public:
        void Run() override
        {
            AmbisonicShelfFilter filter;

            AM_EXPECT_NOT(filter.Configure(0, true, 512, 48000));
            AM_EXPECT_NOT(filter.Configure(4, true, 512, 48000));
            AM_EXPECT_NOT(filter.Configure(5, false, 1024, 44100));
            AM_EXPECT_NOT(filter.Configure(100, true, 512, 48000));
        }
    };

    AM_REGISTER_TEST(ambisonics_shelf_filter, configure_fails_invalid_order);

    AM_TEST_CASE(DSPTestCase, ambisonics_shelf_filter, configure_fails_zero_parameters)
    {
    public:
        void Run() override
        {
            AmbisonicShelfFilter filter;

            AM_EXPECT_NOT(filter.Configure(1, true, 0, 48000));
            AM_EXPECT_NOT(filter.Configure(1, true, 512, 0));
            AM_EXPECT_NOT(filter.Configure(2, false, 0, 0));
        }
    };

    AM_REGISTER_TEST(ambisonics_shelf_filter, configure_fails_zero_parameters);

    AM_TEST_CASE(DSPTestCase, ambisonics_shelf_filter, configure_custom_speed_of_sound)
    {
    public:
        void Run() override
        {
            AmbisonicShelfFilter filter;

            AM_EXPECT(filter.Configure(1, true, 512, 48000, 343.0f));
            ExpectFloatNear(343.0f, filter.GetSpeedOfSound());

            AM_EXPECT(filter.Configure(2, true, 512, 48000, 300.0f));
            ExpectFloatNear(300.0f, filter.GetSpeedOfSound());

            AM_EXPECT(filter.Configure(3, true, 512, 48000, 400.0f));
            ExpectFloatNear(400.0f, filter.GetSpeedOfSound());
        }
    };

    AM_REGISTER_TEST(ambisonics_shelf_filter, configure_custom_speed_of_sound);

    AM_TEST_CASE(DSPTestCase, ambisonics_shelf_filter, configure_channel_count)
    {
    public:
        void Run() override
        {
            AmbisonicShelfFilter filter;

            filter.Configure(1, true, 512, 48000);
            AM_EXPECT_EQ(filter.GetChannelCount(), 4u);

            filter.Configure(1, false, 512, 48000);
            AM_EXPECT_EQ(filter.GetChannelCount(), 3u);

            filter.Configure(2, true, 512, 48000);
            AM_EXPECT_EQ(filter.GetChannelCount(), 9u);

            filter.Configure(2, false, 512, 48000);
            AM_EXPECT_EQ(filter.GetChannelCount(), 5u);

            filter.Configure(3, true, 512, 48000);
            AM_EXPECT_EQ(filter.GetChannelCount(), 16u);

            filter.Configure(3, false, 512, 48000);
            AM_EXPECT_EQ(filter.GetChannelCount(), 7u);
        }
    };

    AM_REGISTER_TEST(ambisonics_shelf_filter, configure_channel_count);
} // namespace SparkyStudios::Audio::Amplitude::Tests
