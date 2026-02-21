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
    AM_TEST_CASE(PureUnitTestCase, math_spherical_position, handles_degrees_coordinates)
    {
    public:
        void Run() override
        {
            constexpr auto azimuth = 45.0f;
            constexpr auto elevation = 30.0f;
            constexpr auto radius = 5.0f;

            const auto sphericalPosition = SphericalPosition::FromDegrees(azimuth, elevation, radius);

            AM_EXPECT_EQ(sphericalPosition.GetAzimuth(), azimuth * AM_DegToRad);
            AM_EXPECT_EQ(sphericalPosition.GetElevation(), elevation * AM_DegToRad);
            AM_EXPECT_EQ(sphericalPosition.GetRadius(), radius);
        }
    };

    AM_REGISTER_TEST(math_spherical_position, handles_degrees_coordinates);
} // namespace SparkyStudios::Audio::Amplitude::Tests
