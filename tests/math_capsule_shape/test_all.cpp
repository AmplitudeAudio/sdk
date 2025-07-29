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
#include "TestUtils.h"

using namespace SparkyStudios::Audio::Amplitude;

void SimpleTestCase::Run()
{
    CapsuleShape shape(50, 100);

    {
        CapsuleShape other(50, 200);
        CapsuleShape some(50, 100);
        CapsuleShape clone(50, 100);
        clone.SetLocation({ 100, 100, 100 });

        AM_EXPECT_EQ(shape, some);
        AM_EXPECT_NE(shape, other);
        AM_EXPECT_NE(shape, clone);
    }

    constexpr AmVector3 inner = { 0, 0, 75 };
    constexpr AmVector3 outer = { 0, 150, 0 };
    constexpr AmVector3 center = { 0, 0, 0 };
    constexpr AmVector3 surfacePoint = { 50, 0, 50 };

    AM_EXPECT_EQ(shape.GetRadius(), 50);
    AM_EXPECT_EQ(shape.GetDiameter(), 100);
    AM_EXPECT_EQ(shape.GetHalfHeight(), 100);
    AM_EXPECT_EQ(shape.GetHeight(), 200);

    AM_EXPECT_EQ(shape.GetShortestDistanceToEdge(inner), 25.0f);
    AM_EXPECT_EQ(shape.GetShortestDistanceToEdge(outer), -100.0f);
    AM_EXPECT_EQ(shape.GetShortestDistanceToEdge(surfacePoint), 0.0f);

    AM_EXPECT(shape.Contains(inner));
    AM_EXPECT_NOT(shape.Contains(outer));
    AM_EXPECT(shape.Contains(surfacePoint));

    {
        shape.SetRadius(100);

        AM_EXPECT_EQ(shape.GetRadius(), 100);
        AM_EXPECT_EQ(shape.GetDiameter(), 200);
    }

    {
        shape.SetHalfHeight(50);

        AM_EXPECT_EQ(shape.GetHalfHeight(), 50);
        AM_EXPECT_EQ(shape.GetHeight(), 100);
    }
}
