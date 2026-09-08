// Copyright (c) 2026-present Sparky Studios. All rights reserved.
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
    AM_TEST_CASE(PureUnitTestCase, math_orientation, extracts_correct_zyx_angles)
    {
    public:
        void Run() override
        {
            auto angleNear = [](AmReal32 a, AmReal32 b) -> bool
            {
                AmReal32 d = std::fmod(a - b, 2.0f * AM_PI32);
                if (d > AM_PI32)
                    d -= 2.0f * AM_PI32;
                if (d < -AM_PI32)
                    d += 2.0f * AM_PI32;
                return std::abs(d) < 1e-3f;
            };

            // (yaw, pitch, roll) cases. |pitch| kept below pi/2 to avoid the ZYX gimbal.
            const AmVector3 cases[] = {
                { 0.0f, 0.0f, 0.0f },  { 0.7f, 0.0f, 0.0f },  { 0.0f, 0.4f, 0.0f }, { 0.0f, 0.0f, 0.5f },
                { 0.7f, 0.4f, 0.5f },  { -1.2f, 0.3f, -0.8f }, { 2.9f, -0.2f, 1.4f },
            };

            for (const auto& c : cases)
            {
                const Orientation reference(c.x, c.y, c.z);

                // Rebuild from basis vectors.
                const Orientation fromBasis(reference.GetForward(), reference.GetUp());
                AM_EXPECT(angleNear(fromBasis.GetYaw(), c.x));
                AM_EXPECT(angleNear(fromBasis.GetPitch(), c.y));
                AM_EXPECT(angleNear(fromBasis.GetRoll(), c.z));

                // Rebuild from the quaternion.
                const Orientation fromQuat(reference.GetQuaternion());
                AM_EXPECT(angleNear(fromQuat.GetYaw(), c.x));
                AM_EXPECT(angleNear(fromQuat.GetPitch(), c.y));
                AM_EXPECT(angleNear(fromQuat.GetRoll(), c.z));

                // Both rebuilds must reproduce the same rotation matrix.
                const auto expected = Mat3ToEigen(reference.GetRotationMatrix());
                AM_EXPECT(expected.isApprox(Mat3ToEigen(fromBasis.GetRotationMatrix()), 1e-4f));
                AM_EXPECT(expected.isApprox(Mat3ToEigen(fromQuat.GetRotationMatrix()), 1e-4f));
            }
        }
    };

    AM_REGISTER_TEST(math_orientation, extracts_correct_zyx_angles);
} // namespace SparkyStudios::Audio::Amplitude::Tests
