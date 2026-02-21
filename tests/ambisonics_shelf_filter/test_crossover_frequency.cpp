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
    AM_TEST_CASE(DSPTestCase, ambisonics_shelf_filter, crossover_frequency_basic)
    {
    public:
        void Run() override
        {
            AmbisonicShelfFilter filter;
            filter.Configure(1, true, 512, 48000, 343.0f);

            AmReal32 crossover = filter.GetCrossoverFrequency();

            AM_EXPECT(crossover > 0.0f);
            AM_EXPECT(crossover < 10000.0f);
            AM_EXPECT(crossover > 100.0f);
        }
    };

    AM_REGISTER_TEST(ambisonics_shelf_filter, crossover_frequency_basic);

    AM_TEST_CASE(DSPTestCase, ambisonics_shelf_filter, crossover_frequency_increases_with_order)
    {
    public:
        void Run() override
        {
            AmbisonicShelfFilter filter1;
            filter1.Configure(1, true, 512, 48000, 343.0f);
            AmReal32 crossover1 = filter1.GetCrossoverFrequency();

            AmbisonicShelfFilter filter2;
            filter2.Configure(2, true, 512, 48000, 343.0f);
            AmReal32 crossover2 = filter2.GetCrossoverFrequency();

            AmbisonicShelfFilter filter3;
            filter3.Configure(3, true, 512, 48000, 343.0f);
            AmReal32 crossover3 = filter3.GetCrossoverFrequency();

            AM_EXPECT(crossover2 > crossover1);
            AM_EXPECT(crossover3 > crossover2);
        }
    };

    AM_REGISTER_TEST(ambisonics_shelf_filter, crossover_frequency_increases_with_order);

    AM_TEST_CASE(DSPTestCase, ambisonics_shelf_filter, crossover_frequency_depends_on_speed_of_sound)
    {
    public:
        void Run() override
        {
            AmbisonicShelfFilter filter1;
            filter1.Configure(1, true, 512, 48000, 300.0f);
            AmReal32 crossover1 = filter1.GetCrossoverFrequency();

            AmbisonicShelfFilter filter2;
            filter2.Configure(1, true, 512, 48000, 343.0f);
            AmReal32 crossover2 = filter2.GetCrossoverFrequency();

            AmbisonicShelfFilter filter3;
            filter3.Configure(1, true, 512, 48000, 400.0f);
            AmReal32 crossover3 = filter3.GetCrossoverFrequency();

            AM_EXPECT(crossover2 > crossover1);
            AM_EXPECT(crossover3 > crossover2);

            AmReal32 ratio1 = crossover2 / crossover1;
            AmReal32 ratio2 = crossover3 / crossover2;
            ExpectFloatNear(ratio1, ratio2, 0.1f);
        }
    };

    AM_REGISTER_TEST(ambisonics_shelf_filter, crossover_frequency_depends_on_speed_of_sound);
} // namespace SparkyStudios::Audio::Amplitude::Tests
