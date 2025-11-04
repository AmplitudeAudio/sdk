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

using namespace SparkyStudios::Audio::Amplitude;

void SimpleTestCase::Run()
{
    // Create a quaternion representing a 90-degree rotation around Z-axis
    const AmQuaternion quaternion = FromAxisAngle(kVector3UnitZ, AM_DegToRad * 90.0f);

    const Orientation orientation(quaternion);

    // it should store the quaternion
    {
        const AmQuaternion storedQuaternion = orientation.GetQuaternion();

        AM_EXPECT(std::abs(storedQuaternion.w - quaternion.w) < kEpsilon);
        AM_EXPECT(std::abs(storedQuaternion.x - quaternion.x) < kEpsilon);
        AM_EXPECT(std::abs(storedQuaternion.y - quaternion.y) < kEpsilon);
        AM_EXPECT(std::abs(storedQuaternion.z - quaternion.z) < kEpsilon);
    }

    // it should compute corresponding forward and up vectors
    {
        const auto forward = orientation.GetForward();
        const auto up = orientation.GetUp();

        // Vectors should be normalized
        AM_EXPECT(std::abs(Length(forward) - 1.0f) < kEpsilon);
        AM_EXPECT(std::abs(Length(up) - 1.0f) < kEpsilon);

        // For a 90-degree Z rotation, forward should be rotated from unit Y
        const AmVector3 expectedForward = RotateVector(kVector3UnitY, quaternion);
        const AmVector3 expectedUp = RotateVector(kVector3UnitZ, quaternion);

        AM_EXPECT_EQ(forward, expectedForward);
        AM_EXPECT_EQ(up, expectedUp);
    }
}
