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
    AM_TEST_CASE(PureUnitTestCase, math_utils, approximate_rational_bounds_terms)
    {
    public:
        void Run() override
        {
            constexpr AmUInt64 maxTerm = 1170;

            // Pitch path: AmplimixImpl::UpdatePitch feeds SetSampleRate(s, 1000).
            bool pitchPathBounded = true;
            for (AmUInt64 s = 1; s <= 4000 && pitchPathBounded; ++s)
            {
                const AmRational r = ApproximateRational(1000, s, maxTerm);
                pitchPathBounded = r.numerator > 0 && r.denominator > 0 && r.numerator <= maxTerm && r.denominator <= maxTerm;
            }
            AM_EXPECT(pitchPathBounded);

            // File path: every integer source rate against the two common engine rates, both directions.
            bool filePathBounded = true;
            for (AmUInt64 rate = 8000; rate <= 192000 && filePathBounded; ++rate)
            {
                const AmRational up48 = ApproximateRational(48000, rate, maxTerm);
                const AmRational down48 = ApproximateRational(rate, 48000, maxTerm);
                const AmRational up441 = ApproximateRational(44100, rate, maxTerm);
                const AmRational down441 = ApproximateRational(rate, 44100, maxTerm);

                filePathBounded = up48.numerator <= maxTerm && up48.denominator <= maxTerm && up48.denominator > 0 &&
                    down48.numerator <= maxTerm && down48.denominator <= maxTerm && down48.denominator > 0 &&
                    up441.numerator <= maxTerm && up441.denominator <= maxTerm && up441.denominator > 0 &&
                    down441.numerator <= maxTerm && down441.denominator <= maxTerm && down441.denominator > 0;
            }
            AM_EXPECT(filePathBounded);
        }
    };

    AM_REGISTER_TEST(math_utils, approximate_rational_bounds_terms);
} // namespace SparkyStudios::Audio::Amplitude::Tests
