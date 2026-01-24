// Copyright (c) 2021-present Sparky Studios. All rights reserved.
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

#include "SimpleTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(SimpleTestCase, math_utils, floating_point_to_fixed_point_conversion)
    {
    public:
        void Run() override
        {
            // it can convert floating-point audio sample to fixed-point
        {
            constexpr AmReal32 value = 0.5f;
            const AmInt32 fixedPointValue = AmFloatToFixedPoint(value);
            AM_EXPECT(std::abs(fixedPointValue - 16384) < kEpsilon);
            }

            // it can convert 16-bit integer to 32-bit floating-point
        {
            constexpr AmInt16 fxp = 16384;
            const AmReal32 value = AmInt16ToReal32(fxp);
            AM_EXPECT(std::abs(value - 0.5f) < kEpsilon);
            }

            // it can convert 32-bit integer to 32-bit floating-point
        {
            constexpr AmInt32 value = 16384;
            const AmReal32 fxp = AmInt32ToReal32(value);
            AM_EXPECT(std::abs(fxp - 0.5f) < kEpsilon);
            }

            // it can convert 32-bit floating-point to 16-bit integer
        {
            constexpr AmReal32 value = 0.5f;

            const AmInt16 fxp1 = AmReal32ToInt16(value, false);
            AM_EXPECT((fxp1 - 16384) < kEpsilon);

            const AmInt16 fxp2 = AmReal32ToInt16(value, true);
            AM_EXPECT((fxp2 - 16384) < kEpsilon);
            }
        }
    };

    AM_REGISTER_TEST(math_utils, floating_point_to_fixed_point_conversion);
} // namespace SparkyStudios::Audio::Amplitude::Tests
