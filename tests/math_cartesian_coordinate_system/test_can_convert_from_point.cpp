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
    AM_TEST_CASE(PureUnitTestCase, math_cartesian_coordinate_system, can_convert_from_point)
    {
    public:
        void Run() override
        {
            const auto from = CartesianCoordinateSystem::Default();
            const auto to = CartesianCoordinateSystem::AmbiX();

            constexpr auto point = AmVector3{ 1, 2, 3 };

            const auto convertedPoint = CartesianCoordinateSystem::Convert(point, from, to);
            const auto convertedBackPoint = CartesianCoordinateSystem::Convert(convertedPoint, to, from);

            AM_EXPECT_EQ(convertedPoint, (AmVector3{ 2, -1, 3 }));
            AM_EXPECT_EQ(convertedBackPoint, point);

            const auto converter = CartesianCoordinateSystem::Converter(from, to);

            const auto convertedPoint2 = converter.Forward(point);
            const auto convertedBackPoint2 = converter.Backward(convertedPoint);

            AM_EXPECT_EQ(convertedPoint2, convertedPoint);
            AM_EXPECT_EQ(convertedBackPoint2, point);
        }
    };

    AM_REGISTER_TEST(math_cartesian_coordinate_system, can_convert_from_point);
} // namespace SparkyStudios::Audio::Amplitude::Tests
