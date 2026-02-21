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
    AM_TEST_CASE(PureUnitTestCase, math_spherical_position, handles_cartesian_coordinates)
    {
    public:
        void Run() override
        {
            constexpr AmVector3 cartesianPosition = { 5.0f, 3.0f, 4.0f };
            SphericalPosition position = SphericalPosition::FromWorldSpace(cartesianPosition);

            AM_EXPECT_EQ(position.GetAzimuth(), -std::atan2(cartesianPosition.y, cartesianPosition.x));
            AM_EXPECT_EQ(position.GetElevation(), std::atan2(cartesianPosition.z, Length(cartesianPosition.xy)));
            AM_EXPECT_EQ(position.GetRadius(), Length(cartesianPosition));

        {
            const auto sphericalPosition = SphericalPosition::ForHRTF(cartesianPosition);

            AM_EXPECT_EQ(sphericalPosition.GetAzimuth(), 90.0f * AM_DegToRad - std::atan2(cartesianPosition.y, cartesianPosition.x));
            AM_EXPECT_EQ(sphericalPosition.GetElevation(), std::atan2(cartesianPosition.z, Length(cartesianPosition.xy)));
            AM_EXPECT_EQ(sphericalPosition.GetRadius(), Length(cartesianPosition));
            }
        }
    };

    AM_REGISTER_TEST(math_spherical_position, handles_cartesian_coordinates);
} // namespace SparkyStudios::Audio::Amplitude::Tests
