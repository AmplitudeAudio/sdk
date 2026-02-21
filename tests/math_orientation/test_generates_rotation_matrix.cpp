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
    AM_TEST_CASE(PureUnitTestCase, math_orientation, generates_rotation_matrix)
    {
    public:
        void Run() override
        {
            constexpr AmReal32 yaw = AM_DegToRad * 45.0f;
            constexpr AmReal32 pitch = AM_DegToRad * 30.0f;
            constexpr AmReal32 roll = AM_DegToRad * 15.0f;

            const Orientation orientation(yaw, pitch, roll);
            const AmMatrix3 rotationMatrix = orientation.GetRotationMatrix();

            // it should produce a valid rotation matrix
        {
            // Check if it's orthogonal (R * R^T = I)
            constexpr AmMatrix3 identity = kMatrix3Identity;
            const AmMatrix3 product = Mul(rotationMatrix, Transpose(rotationMatrix));

            for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j)
            AM_EXPECT(std::abs(product[i][j] - identity[i][j]) < kEpsilon);

            // Check determinant is 1 (proper rotation)
            const AmReal32 det = Determinant(rotationMatrix);
            AM_EXPECT(std::abs(det - 1.0f) < kEpsilon);
            }

            // it should correctly transform the unit vectors
        {
            const AmVector3 transformedY = Transform(rotationMatrix, kVector3UnitY);
            const AmVector3 transformedZ = Transform(rotationMatrix, kVector3UnitZ);

            AM_EXPECT_EQ(transformedY, orientation.GetForward());
            AM_EXPECT_EQ(transformedZ, orientation.GetUp());
            }
        }
    };

    AM_REGISTER_TEST(math_orientation, generates_rotation_matrix);
} // namespace SparkyStudios::Audio::Amplitude::Tests
