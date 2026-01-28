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
    AM_TEST_CASE(SimpleTestCase, math_cartesian_coordinate_system, can_convert_from_scalar)
    {
    public:
        void Run() override
        {
            const auto from = CartesianCoordinateSystem::Default();
            const auto to = CartesianCoordinateSystem::AmbiX();

            constexpr auto scalar = 5.0f;

            const auto convertedScalar = CartesianCoordinateSystem::Convert(scalar, from, to);
            const auto convertedBackScalar = CartesianCoordinateSystem::Convert(convertedScalar, to, from);

            AM_EXPECT_EQ(convertedScalar, scalar);
            AM_EXPECT_EQ(convertedBackScalar, scalar);

            const auto converter = CartesianCoordinateSystem::Converter(from, to);

            const auto convertedScalar2 = converter.Forward(scalar);
            const auto convertedBackScalar2 = converter.Backward(convertedScalar);

            AM_EXPECT_EQ(convertedScalar2, scalar);
            AM_EXPECT_EQ(convertedBackScalar2, scalar);
        }
    };

    AM_REGISTER_TEST(math_cartesian_coordinate_system, can_convert_from_scalar);
} // namespace SparkyStudios::Audio::Amplitude::Tests
