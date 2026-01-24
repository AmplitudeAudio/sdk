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
#include "TestUtils.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(SimpleTestCase, math_box_shape, all)
    {
    public:
        void Run() override
        {
            BoxShape shape(50, 50, 50);

        {
            BoxShape other(250, 250, 250);
            BoxShape some(50, 50, 50);
            BoxShape clone({ 100, 100, 100 }, { 100, 100, 100 });

            AM_EXPECT_EQ(shape, some);
            AM_EXPECT_NE(shape, other);
            AM_EXPECT_NE(shape, clone);
            }

            constexpr AmVector3 inner = { -25, -25, -25 };
            constexpr AmVector3 outer = { 100, 100, 100 };
            constexpr AmVector3 center = { 0, 0, 0 };
            constexpr AmVector3 surfacePoint = { 0, 0, 50 };

            AM_EXPECT_EQ(shape.GetHalfWidth(), 50);
            AM_EXPECT_EQ(shape.GetHalfHeight(), 50);
            AM_EXPECT_EQ(shape.GetHalfDepth(), 50);

            AM_EXPECT_EQ(shape.GetWidth(), 100);
            AM_EXPECT_EQ(shape.GetHeight(), 100);
            AM_EXPECT_EQ(shape.GetDepth(), 100);

            const auto corners = shape.GetCorners();

            AM_EXPECT_EQ(shape.GetShortestDistanceToEdge(inner), 25.0f);
            AM_EXPECT_EQ(shape.GetShortestDistanceToEdge(outer), -50.0f);
            AM_EXPECT_EQ(shape.GetShortestDistanceToEdge(center), 50.0f);
            AM_EXPECT_EQ(shape.GetShortestDistanceToEdge(surfacePoint), 0.0f);

            AM_EXPECT(shape.Contains(inner));
            AM_EXPECT_NOT(shape.Contains(outer));
            AM_EXPECT(shape.Contains(surfacePoint));

            AM_EXPECT_EQ(shape.GetClosestPoint(inner), inner);
            AM_EXPECT_EQ(shape.GetClosestPoint(outer), corners[4]);
            AM_EXPECT_EQ(shape.GetClosestPoint(center), center);
            AM_EXPECT_EQ(shape.GetClosestPoint(surfacePoint), (AmVector3{ 0, 0, 50 }));

        {
            shape.SetHalfWidth(10);
            shape.SetHalfHeight(20);
            shape.SetHalfDepth(30);

            AM_EXPECT_EQ(shape.GetHalfWidth(), 10);
            AM_EXPECT_EQ(shape.GetHalfHeight(), 20);
            AM_EXPECT_EQ(shape.GetHalfDepth(), 30);

            AM_EXPECT_EQ(shape.GetWidth(), 20);
            AM_EXPECT_EQ(shape.GetHeight(), 40);
            AM_EXPECT_EQ(shape.GetDepth(), 60);
            }

        {
            constexpr AmVector3 location = { 100, 100, 100 };
            shape.SetLocation(location);

            AM_EXPECT_EQ(shape.GetLocation(), location);
            }

        {
            const Orientation orientation = Orientation(FromAxisAngle({ 0.5, 0.5, 0.5 }, AM_PI32));
            shape.SetOrientation(orientation);

            AM_EXPECT_EQ(std::memcmp(&shape.GetOrientation(), &orientation, sizeof(Orientation)), 0);
            }
        }
    };

    AM_REGISTER_TEST(math_box_shape, all);
} // namespace SparkyStudios::Audio::Amplitude::Tests
