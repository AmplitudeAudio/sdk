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
    AM_TEST_CASE(PureUnitTestCase, math_spherical_position, handles_polar_coordinates)
    {
    public:
        void Run() override
        {
            SphericalPosition position(AM_DegToRad * 45.0f, AM_DegToRad * 30.0f, 5.0f);

        {
            const auto otherPosition = SphericalPosition(AM_DegToRad * 45.0f, AM_DegToRad * 30.0f, 5.0f);
            const auto differentPosition = SphericalPosition(AM_DegToRad * 60.0f, AM_DegToRad * 45.0f, 5.0f);

            AM_EXPECT_EQ(position, position);
            AM_EXPECT_EQ(position, otherPosition);
            AM_EXPECT_NE(position, differentPosition);
            }

        {
            AM_EXPECT_EQ(position.GetAzimuth(), AM_DegToRad * 45.0f);
            AM_EXPECT_EQ(position.GetElevation(), AM_DegToRad * 30.0f);
            AM_EXPECT_EQ(position.GetRadius(), 5.0f);
            }

        {
            const auto cartesianPosition = position.ToCartesian();

            AM_EXPECT_EQ(cartesianPosition.x, +5.0f * std::cos(position.GetElevation()) * std::cos(position.GetAzimuth()));
            AM_EXPECT_EQ(cartesianPosition.y, -5.0f * std::cos(position.GetElevation()) * std::sin(position.GetAzimuth()));
            AM_EXPECT_EQ(cartesianPosition.z, +5.0f * std::sin(position.GetElevation()));
            }

        {
            const auto flippedPosition = position.FlipAzimuth();

            AM_EXPECT_EQ(flippedPosition.GetAzimuth(), -45.0f * AM_DegToRad);
            AM_EXPECT_EQ(flippedPosition.GetElevation(), position.GetElevation());
            AM_EXPECT_EQ(flippedPosition.GetRadius(), position.GetRadius());
            }

        {
            position.SetAzimuth(AM_DegToRad * 60.0f);
            position.SetElevation(AM_DegToRad * 45.0f);
            position.SetRadius(10.0f);

            AM_EXPECT_EQ(position.GetAzimuth(), AM_DegToRad * 60.0f);
            AM_EXPECT_EQ(position.GetElevation(), AM_DegToRad * 45.0f);
            AM_EXPECT_EQ(position.GetRadius(), 10.0f);
            }

        {
            const auto rotation = FromAxisAngle(kVector3UnitZ, AM_DegToRad * 90.0f);
            const auto rotatedPosition = position.Rotate(rotation);

            const auto rotatedPosition2 = SphericalPosition::FromWorldSpace(RotateVector(position.ToCartesian(), rotation));

            AM_EXPECT_EQ(rotatedPosition.GetAzimuth(), rotatedPosition2.GetAzimuth());
            AM_EXPECT_EQ(rotatedPosition.GetElevation(), rotatedPosition2.GetElevation());
            AM_EXPECT_EQ(rotatedPosition.GetRadius(), rotatedPosition2.GetRadius());
            }
        }
    };

    AM_REGISTER_TEST(math_spherical_position, handles_polar_coordinates);
} // namespace SparkyStudios::Audio::Amplitude::Tests
