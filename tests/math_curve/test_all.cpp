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

#include "EngineTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(EngineTestCase, math_curve, all)
    {
    public:
        void Run() override
        {
            CurvePoint zero{ 0.0, 0.0f };
            CurvePoint middle{ 0.5, 1.0f };
            CurvePoint one{ 1.0, 0.0f };

            CurvePart part1;
            part1.SetStart(zero);
            part1.SetEnd(middle);
            part1.SetFader("Linear");

            CurvePart part2;
            part2.SetStart(middle);
            part2.SetEnd(one);
            part2.SetFader("Linear");

            Curve curve;

            // cannot be initialized with a null definition
        {
            curve.Initialize(nullptr);

            AM_EXPECT(curve.Get(0.0) == 0.0f);
            }

            // can be initialized with curve parts
        {
            const std::vector<CurvePart> parts = { part1, part2 };
            curve.Initialize(parts);

            AM_EXPECT(curve.Get(0.0) - 0.0f < kEpsilon);
            AM_EXPECT(curve.Get(0.5) - 1.0f < kEpsilon);
            AM_EXPECT(curve.Get(1.0) - 0.0f < kEpsilon);
            }

            // can get the right values
        {
            curve.Initialize({ part1, part2 });

            for (AmReal64 t = 0.0; t <= 0.5; t += 0.1)
        {
            const AmReal32 value = curve.Get(t);
            AM_EXPECT(value - (t * 2) < kEpsilon);
            }

            for (AmReal64 t = 0.5; t <= 1.0; t += 0.1)
        {
            const AmReal32 value = curve.Get(t);
            AM_EXPECT(value - 2 - 2 * t < kEpsilon);
            }
            }

            // cannot get values outside the range
        {
            curve.Initialize({ part1, part2 });

            AM_EXPECT(curve.Get(-1.0) == 0.0f);
            AM_EXPECT(curve.Get(2.0) == 0.0f);
            }
        }
    };

    AM_REGISTER_TEST(math_curve, all);
} // namespace SparkyStudios::Audio::Amplitude::Tests
