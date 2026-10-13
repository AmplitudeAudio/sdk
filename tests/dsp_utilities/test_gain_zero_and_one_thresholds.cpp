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

#include <DSP/Gain.h>

#include "DSPTestCase.h"
#include "TestRegistry.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    // Gains within 0.001 of 0 (or of 1) are treated as exactly 0 (or 1) so the mixer can skip or copy.
    AM_TEST_CASE(DSPTestCase, dsp_utilities, gain_zero_and_one_thresholds)
    {
    public:
        void Run() override
        {
            AM_EXPECT(Gain::IsZero(0.0f));
            AM_EXPECT(Gain::IsZero(0.0005f));
            AM_EXPECT(Gain::IsZero(-0.0005f));
            AM_EXPECT_NOT(Gain::IsZero(0.002f));
            AM_EXPECT_NOT(Gain::IsZero(-0.002f));
            AM_EXPECT_NOT(Gain::IsZero(1.0f));

            AM_EXPECT(Gain::IsOne(1.0f));
            AM_EXPECT(Gain::IsOne(0.9995f));
            AM_EXPECT(Gain::IsOne(1.0005f));
            AM_EXPECT_NOT(Gain::IsOne(0.998f));
            AM_EXPECT_NOT(Gain::IsOne(1.002f));
            AM_EXPECT_NOT(Gain::IsOne(0.0f));
        }
    };

    AM_REGISTER_TEST(dsp_utilities, gain_zero_and_one_thresholds);
} // namespace SparkyStudios::Audio::Amplitude::Tests
