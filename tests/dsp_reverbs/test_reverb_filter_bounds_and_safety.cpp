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
#include <Utils/Audio/Reverb/BaseReverb.h>
#include <Utils/Audio/Reverb/ReverbFilter.h>

#include "DSPTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    class TestableBaseReverb : public BaseReverb
    {
    public:
        explicit TestableBaseReverb(AmUInt64 sampleRate)
            : BaseReverb(sampleRate)
        {}

        using BaseReverb::_arrayAllPass;
        using BaseReverb::_arrayLowPass;
        using BaseReverb::_arrayTwo;
        using BaseReverb::_earlyRef;
        using BaseReverb::kNumFilters;
        using BaseReverb::LimitNumFilters;
    };

    AM_TEST_CASE(DSPTestCase, dsp_reverbs, reverb_filter_bounds_and_safety)
    {
    public:
        void Run() override
        {
            // 1. ReverbFilters::Init resets _delayIndex = 0
            ReverbFilters filter;
            filter.Init(48000, 100);
            filter._delayIndex = 42;
            filter.Init(48000, 100);
            AM_EXPECT_EQ(filter._delayIndex, 0);

            // 2. Bounds guarding on oversized delay requests
            // Request size 500 on a 100-sample filter: should clamp to buffer size (100)
            filter.CombFeedBack(1.0f, 500, 0.5f);
            AM_EXPECT_EQ(filter._delaySize, 100);

            filter.LowPassCombFeedBack(1.0f, 500, 0.5f, 0.2f);
            AM_EXPECT_EQ(filter._delaySize, 100);

            filter.AllPass(1.0f, 500);
            AM_EXPECT_EQ(filter._delaySize, 100);

            filter.OneTap(1.0f, 500);
            AM_EXPECT_EQ(filter._delaySize, 100);

            // Bounds guarding on zero or negative size
            filter.CombFeedBack(1.0f, 0, 0.5f);
            AM_EXPECT_EQ(filter._delaySize, 1);

            filter.CombFeedBack(1.0f, -10, 0.5f);
            AM_EXPECT_EQ(filter._delaySize, 1);

            // 3. LimitNumFilters accesses all 32 filters
            AmUInt32 num = 32;
            TestableBaseReverb::LimitNumFilters(num, 0);
            AM_EXPECT_EQ(num, 32u);

            num = 16;
            TestableBaseReverb::LimitNumFilters(num, 16);
            AM_EXPECT_EQ(num, 32u);

            num = 40;
            TestableBaseReverb::LimitNumFilters(num, 0);
            AM_EXPECT_EQ(num, 32u);

            // 4. Memory bounds sizing at 48kHz
            TestableBaseReverb reverb(48000);
            AM_EXPECT_EQ(reverb._arrayAllPass[0]._delayLines.size(), 1000u);
            AM_EXPECT_EQ(reverb._arrayLowPass[0]._delayLines.size(), 1u);
            AM_EXPECT_EQ(reverb._arrayTwo[0]._delayLines.size(), 2500u);
            AM_EXPECT_EQ(reverb._earlyRef._delayLines.size(), 4800u);
        }
    };

    AM_REGISTER_TEST(dsp_reverbs, reverb_filter_bounds_and_safety);
} // namespace SparkyStudios::Audio::Amplitude::Tests
