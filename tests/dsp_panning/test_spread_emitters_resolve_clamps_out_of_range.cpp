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

#include <limits>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <DSP/SpreadEmitters.h>

#include "PureUnitTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(PureUnitTestCase, dsp_panning, spread_emitters_resolve_clamps_out_of_range)
    {
    public:
        void Run() override
        {
            const auto& s51 = ChannelLayout::Get(eChannelLayout_Surround_5_1);

            AM_EXPECT_EQ(220.0f, SpreadEmitters::Resolve(kAmSpreadNative, s51));
            AM_EXPECT_EQ(0.0f, SpreadEmitters::Resolve(-0.5f, s51));
            AM_EXPECT_EQ(0.0f, SpreadEmitters::Resolve(-200.0f, s51));
            AM_EXPECT_EQ(360.0f, SpreadEmitters::Resolve(400.0f, s51));
            AM_EXPECT_EQ(90.0f, SpreadEmitters::Resolve(90.0f, s51));
            AM_EXPECT_EQ(0.0f, SpreadEmitters::Resolve(std::numeric_limits<AmReal32>::quiet_NaN(), s51));
        }
    };

    AM_REGISTER_TEST(dsp_panning, spread_emitters_resolve_clamps_out_of_range);
} // namespace SparkyStudios::Audio::Amplitude::Tests
