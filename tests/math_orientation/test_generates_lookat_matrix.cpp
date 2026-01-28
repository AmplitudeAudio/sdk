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
    AM_TEST_CASE(SimpleTestCase, math_orientation, generates_lookat_matrix)
    {
    public:
        void Run() override
        {
            const Orientation orientation(0.0f, 0.0f, 0.0f); // Zero orientation
            constexpr AmVector3 eye = { 5.0f, 5.0f, 5.0f };

            const AmMatrix4 lookAtMatrix = orientation.GetLookAtMatrix(eye);

            // it should produce a valid transformation matrix
        {
            // The matrix should be invertible
            const AmReal32 det = Determinant(lookAtMatrix);
            AM_EXPECT(std::abs(det) > kEpsilon);
            }

            // it should incorporate the eye position
        {
            // The translation part should be related to the eye position
            AM_EXPECT(((lookAtMatrix[3][0] != 0.0f) || (lookAtMatrix[3][1] != 0.0f) || (lookAtMatrix[3][2] != 0.0f)));
            }
        }
    };

    AM_REGISTER_TEST(math_orientation, generates_lookat_matrix);
} // namespace SparkyStudios::Audio::Amplitude::Tests
