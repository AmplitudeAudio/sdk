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

using namespace SparkyStudios::Audio::Amplitude;

void SimpleTestCase::Run()
{
    SphereShape shape(50);

    {
        SphereShape other(100);
        SphereShape some(50);
        SphereShape clone(50);
        clone.SetLocation({ 100, 100, 100 });

        AM_EXPECT(shape == some);
        AM_EXPECT(shape != other);
        AM_EXPECT_NOT(shape == clone);
    }

    constexpr AmVector3 inner = { 25, 25, 25 };
    constexpr AmVector3 outer = { 100, 100, 100 };
    constexpr AmVector3 center = { 0, 0, 0 };
    constexpr AmVector3 surfacePoint = { 0, 0, 50 };

    AM_EXPECT(shape.GetRadius() == 50);
    AM_EXPECT(shape.GetDiameter() == 100);

    AM_EXPECT(shape.GetShortestDistanceToEdge(inner) == shape.GetRadius() - Length(inner));
    AM_EXPECT(shape.GetShortestDistanceToEdge(outer) == shape.GetRadius() - Length(outer));
    AM_EXPECT(shape.GetShortestDistanceToEdge(surfacePoint) == 0.0f);

    AM_EXPECT(shape.Contains(inner));
    AM_EXPECT_NOT(shape.Contains(outer));
    AM_EXPECT(shape.Contains(surfacePoint));

    {
        shape.SetRadius(100);

        AM_EXPECT(shape.GetRadius() == 100);
        AM_EXPECT(shape.GetDiameter() == 200);
    }

    {
        constexpr AmVector3 location = { 100, 100, 100 };
        shape.SetLocation(location);

        AM_EXPECT(shape.GetLocation() == location);
    }

    {
        const Orientation orientation = Orientation(FromAxisAngle({ 0.5, 0.5, 0.5 }, AM_PI32));
        shape.SetOrientation(orientation);

        AM_EXPECT(std::memcmp(&shape.GetOrientation(), &orientation, sizeof(Orientation)) == 0);
    }
}
