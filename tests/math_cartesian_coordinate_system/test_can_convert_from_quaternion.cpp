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
    AM_TEST_CASE(SimpleTestCase, math_cartesian_coordinate_system, can_convert_from_quaternion)
    {
    public:
        void Run() override
        {
            const auto from = CartesianCoordinateSystem::Default();
            const auto to = CartesianCoordinateSystem::AmbiX();

            const auto rotation = FromAxisAngle(kVector3UnitY, AM_DegToRad * 45.0f);

            const auto convertedRotation = CartesianCoordinateSystem::Convert(rotation, from, to);
            const auto convertedBackRotation = CartesianCoordinateSystem::Convert(convertedRotation, to, from);

            AM_EXPECT_EQ(convertedRotation, (AmQuaternion{ rotation.w, rotation.y, -rotation.x, rotation.z }));
            AM_EXPECT_EQ(convertedBackRotation, rotation);

            const auto converter = CartesianCoordinateSystem::Converter(from, to);

            const auto convertedRotation2 = converter.Forward(rotation);
            const auto convertedBackRotation2 = converter.Backward(convertedRotation);

            AM_EXPECT_EQ(convertedRotation2, convertedRotation);
            AM_EXPECT_EQ(convertedBackRotation2, rotation);
        }
    };

    AM_REGISTER_TEST(math_cartesian_coordinate_system, can_convert_from_quaternion);
} // namespace SparkyStudios::Audio::Amplitude::Tests
