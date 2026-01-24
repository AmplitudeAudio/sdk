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
    AM_TEST_CASE(SimpleTestCase, math_orientation, constructs_from_euler_angles)
    {
    public:
        void Run() override
        {
            constexpr AmReal32 yaw = AM_DegToRad * 45.0f;
            constexpr AmReal32 pitch = AM_DegToRad * 30.0f;
            constexpr AmReal32 roll = AM_DegToRad * 15.0f;

            const Orientation orientation(yaw, pitch, roll);

            // it should store the correct Euler angles
        {
            AM_EXPECT(std::abs(orientation.GetYaw() - yaw) < kEpsilon);
            AM_EXPECT(std::abs(orientation.GetPitch() - pitch) < kEpsilon);
            AM_EXPECT(std::abs(orientation.GetRoll() - roll) < kEpsilon);
            }

            // it should compute forward and up vectors
        {
            const auto forward = orientation.GetForward();
            const auto up = orientation.GetUp();

            // Forward and up vectors should be normalized
            AM_EXPECT(std::abs(Length(forward) - 1.0f) < kEpsilon);
            AM_EXPECT(std::abs(Length(up) - 1.0f) < kEpsilon);

            // Forward and up vectors should be perpendicular
            AM_EXPECT(std::abs(Dot(forward, up)) < kEpsilon);

            // Expected values for yaw=45°, pitch=30°, roll=15°
            // Original forward is +Y, up is +Z, so ZYX order: Rz(yaw) * Ry(pitch) * Rx(roll)
            // These values are computed for that convention.
            constexpr AmVector3 expectedForward = {
            -0.591506f, // Forward X component
            0.774519f, // Forward Y component
            0.224144f // Forward Z component
            };

            constexpr AmVector3 expectedUp = {
            0.524519f, // Up X component
            0.158494f, // Up Y component
            0.836516f // Up Z component
            };

            // Verify forward vector components with appropriate tolerance
            AM_EXPECT(std::abs(forward.x - expectedForward.x) < kEpsilon);
            AM_EXPECT(std::abs(forward.y - expectedForward.y) < kEpsilon);
            AM_EXPECT(std::abs(forward.z - expectedForward.z) < kEpsilon);

            // Verify up vector components with appropriate tolerance
            AM_EXPECT(std::abs(up.x - expectedUp.x) < kEpsilon);
            AM_EXPECT(std::abs(up.y - expectedUp.y) < kEpsilon);
            AM_EXPECT(std::abs(up.z - expectedUp.z) < kEpsilon);
            }

            // it should compute ZYZ Euler angles
        {
            // Alpha, beta, gamma should be computed
            const auto alpha = orientation.GetAlpha();
            const auto beta = orientation.GetBeta();
            const auto gamma = orientation.GetGamma();

            // These values should be within valid ranges
            AM_EXPECT(alpha >= -AM_PI32);
            AM_EXPECT(alpha <= AM_PI32);
            AM_EXPECT(beta >= 0.0f);
            AM_EXPECT(beta <= AM_PI32);
            AM_EXPECT(gamma >= -AM_PI32);
            AM_EXPECT(gamma <= AM_PI32);
            }

            // it should compute a valid quaternion
        {
            const auto quaternion = orientation.GetQuaternion();

            // Quaternion should be normalized
            const AmReal32 length = Length(quaternion);

            AM_EXPECT(std::abs(length - 1.0f) < kEpsilon);
            }

            // it should handle extreme angle values
        {
            // Test with large angles
            const Orientation orientation1(AM_PI32 * 2.5f, AM_PI32 * 1.5f, AM_PI32 * 3.0f);

            // Vectors should still be normalized
            AM_EXPECT(std::abs(Length(orientation1.GetForward()) - 1.0f) < kEpsilon);
            AM_EXPECT(std::abs(Length(orientation1.GetUp()) - 1.0f) < kEpsilon);
            }
        }
    };

    AM_REGISTER_TEST(math_orientation, constructs_from_euler_angles);
} // namespace SparkyStudios::Audio::Amplitude::Tests
