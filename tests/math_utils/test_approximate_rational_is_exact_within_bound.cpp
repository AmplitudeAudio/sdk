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

#include "PureUnitTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(PureUnitTestCase, math_utils, approximate_rational_is_exact_within_bound)
    {
    public:
        void Run() override
        {
            constexpr AmUInt64 maxTerm = 1170;

            // 48000 / 44100 reduces to 160 / 147, which fits the bound: it must be returned untouched.
            const AmRational standard = ApproximateRational(48000, 44100, maxTerm);
            AM_EXPECT(standard.exact);
            AM_EXPECT_EQ(standard.numerator, 160ULL);
            AM_EXPECT_EQ(standard.denominator, 147ULL);

            // Identity ratios reduce to 1 / 1.
            const AmRational identity = ApproximateRational(48000, 48000, maxTerm);
            AM_EXPECT(identity.exact);
            AM_EXPECT_EQ(identity.numerator, 1ULL);
            AM_EXPECT_EQ(identity.denominator, 1ULL);

            // 48000 / 44056 reduces to 6000 / 5507, which does not fit: it must be approximated.
            const AmRational snapped = ApproximateRational(48000, 44056, maxTerm);
            AM_EXPECT_NOT(snapped.exact);
            AM_EXPECT_EQ(snapped.numerator, 1071ULL);
            AM_EXPECT_EQ(snapped.denominator, 983ULL);
        }
    };

    AM_REGISTER_TEST(math_utils, approximate_rational_is_exact_within_bound);
} // namespace SparkyStudios::Audio::Amplitude::Tests
