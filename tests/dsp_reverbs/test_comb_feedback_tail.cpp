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
#include <Utils/Audio/Reverb/ReverbFilter.h>

#include "DSPTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(DSPTestCase, dsp_reverbs, comb_feedback_tail)
    {
    public:
        void Run() override
        {
            ReverbFilters filter;
            filter.Init(48000);

            constexpr AmInt32 delay = 100;
            constexpr AmReal32 feedback = 0.8f;

            // Feed single impulse
            AmReal32 y0 = filter.CombFeedBack(1.0f, delay, feedback);
            AM_EXPECT_EQ(y0, 1.0f);

            // Feed zeros and verify echoes at multiples of delay
            int echoCount = 0;
            for (int i = 1; i < 500; ++i)
            {
                AmReal32 y = filter.CombFeedBack(0.0f, delay, feedback);
                if (std::abs(y) > 0.05f)
                    echoCount++;
            }

            // An IIR feedback comb must produce multiple repeating echoes (at least 3 before decaying below 0.05)
            AM_EXPECT(echoCount >= 3);
        }
    };

    AM_REGISTER_TEST(dsp_reverbs, comb_feedback_tail);
} // namespace SparkyStudios::Audio::Amplitude::Tests
