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
#include "SparkyStudios/Audio/Amplitude/Math/Curve.h"

using namespace SparkyStudios::Audio::Amplitude;

void EngineTestCase::Run()
{
    CurvePart part1;

    CurvePoint zero{ 0.0, 0.0f };
    CurvePoint one{ 1.0, 1.0f };

    part1.Initialize(nullptr);
    AM_EXPECT_EQ(part1.GetStart(), zero);
    AM_EXPECT_EQ(part1.GetEnd(), zero);
    AM_EXPECT(part1.GetFader() == nullptr);

    part1.SetStart(one);
    AM_EXPECT_EQ(part1.GetStart(), one);

    part1.SetEnd(one);
    AM_EXPECT_EQ(part1.GetEnd(), one);

    part1.SetFader("Linear");
    AM_EXPECT(part1.GetFader() != nullptr);

    part1.SetFader("Invalid");
    AM_EXPECT(part1.GetFader() == nullptr);

    {
        part1.SetFader("Linear");
        part1.SetStart(zero);
        part1.SetEnd(one);

        for (AmReal64 t = 0.0; t <= 1.0; t += 0.1)
        {
            const AmReal32 value = part1.Get(t);
            AM_EXPECT(value - t < kEpsilon);
        }
    }

    // can be copied
    {
        CurvePart copy1 = part1;

        AM_EXPECT_EQ(copy1.GetStart(), part1.GetStart());
        AM_EXPECT_EQ(copy1.GetEnd(), part1.GetEnd());

        copy1 = copy1;

        AM_EXPECT_EQ(copy1.GetStart(), part1.GetStart());
        AM_EXPECT_EQ(copy1.GetEnd(), part1.GetEnd());
    }
}
