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

#include <Math/LinearAlgebra.h>

#include "SimpleTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(SimpleTestCase, math_barycentric_coordinates, can_be_computed_from_point_inside_triangle)
    {
    public:
        void Run() override
        {
            constexpr AmVector3 point1 = { 1, 2, 1 };
            constexpr AmVector3 point2 = { 0, 0, 0 };
            constexpr AmVector3 point3 = { 2, 1, 2 };

            constexpr auto center = kVector3One;
            constexpr auto triangle = Triangle{ point1, point2, point3 };

            const auto i = Sub(point1, center);
            const auto j = Sub(point2, center);
            const auto k = Cross(i, j);

            constexpr auto rayOrigin = center;
            const auto rayDirection = k;

            BarycentricCoordinates coordinates(center, triangle);

            AM_EXPECT(coordinates.IsValid());

            AM_EXPECT_EQ(coordinates.m_U + coordinates.m_V + coordinates.m_W, 1.0f);

            AM_EXPECT((coordinates.m_U - 1.0f / 3.0f) < kEpsilon);
            AM_EXPECT((coordinates.m_V - 1.0f / 3.0f) < kEpsilon);
            AM_EXPECT((coordinates.m_W - 1.0f / 3.0f) < kEpsilon);
        }
    };

    AM_REGISTER_TEST(math_barycentric_coordinates, can_be_computed_from_point_inside_triangle);
} // namespace SparkyStudios::Audio::Amplitude::Tests
