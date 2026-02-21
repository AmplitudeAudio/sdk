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

#include "PureUnitTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(PureUnitTestCase, math_cartesian_coordinate_system, has_correct_axes)
    {
    public:
        void Run() override
        {
            // Right-handed Z-Up Cartesian Coordinate System
        {
            const CartesianCoordinateSystem coordinateSystem = CartesianCoordinateSystem::RightHandedZUp();

            AM_EXPECT_EQ(coordinateSystem.GetRightVector(), kVector3UnitX);
            AM_EXPECT_EQ(coordinateSystem.GetUpVector(), kVector3UnitZ);
            AM_EXPECT_EQ(coordinateSystem.GetForwardVector(), kVector3UnitY);
            }

            // Left-handed Z-Up Cartesian Coordinate System
        {
            const CartesianCoordinateSystem coordinateSystem = CartesianCoordinateSystem::LeftHandedZUp();

            AM_EXPECT_EQ(coordinateSystem.GetRightVector(), kVector3UnitX);
            AM_EXPECT_EQ(coordinateSystem.GetUpVector(), kVector3UnitZ);
            AM_EXPECT_EQ(coordinateSystem.GetForwardVector(), Negate(kVector3UnitY));
            }

            // Right-handed Y-Up Cartesian Coordinate System
        {
            const CartesianCoordinateSystem coordinateSystem = CartesianCoordinateSystem::RightHandedYUp();

            AM_EXPECT_EQ(coordinateSystem.GetRightVector(), kVector3UnitX);
            AM_EXPECT_EQ(coordinateSystem.GetUpVector(), kVector3UnitY);
            AM_EXPECT_EQ(coordinateSystem.GetForwardVector(), Negate(kVector3UnitZ));
            }

            // Left-handed Y-Up Cartesian Coordinate System
        {
            const CartesianCoordinateSystem coordinateSystem = CartesianCoordinateSystem::LeftHandedYUp();

            AM_EXPECT_EQ(coordinateSystem.GetRightVector(), kVector3UnitX);
            AM_EXPECT_EQ(coordinateSystem.GetUpVector(), kVector3UnitY);
            AM_EXPECT_EQ(coordinateSystem.GetForwardVector(), kVector3UnitZ);
            }

            // Arbitrary Cartesian Coordinate System
        {
            constexpr auto rightVector = CartesianCoordinateSystem::Axis::NegativeX;
            constexpr auto upVector = CartesianCoordinateSystem::Axis::PositiveY;
            constexpr auto forwardVector = CartesianCoordinateSystem::Axis::NegativeZ;

            const CartesianCoordinateSystem coordinateSystem(rightVector, forwardVector, upVector);

            AM_EXPECT_EQ(coordinateSystem.GetRightVector(), CartesianCoordinateSystem::GetVector(rightVector));
            AM_EXPECT_EQ(coordinateSystem.GetUpVector(), CartesianCoordinateSystem::GetVector(upVector));
            AM_EXPECT_EQ(coordinateSystem.GetForwardVector(), CartesianCoordinateSystem::GetVector(forwardVector));
            }
        }
    };

    AM_REGISTER_TEST(math_cartesian_coordinate_system, has_correct_axes);
} // namespace SparkyStudios::Audio::Amplitude::Tests
