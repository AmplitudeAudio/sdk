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
    AM_TEST_CASE(PureUnitTestCase, math_orientation, extracts_correct_zyz_angles)
    {
    public:
        void Run() override
        {
            auto checkZYZ = [&](const Orientation& o)
            {
                // Rz(alpha) * Ry(beta) * Rz(gamma) must reproduce the rotation matrix.
                const auto a = Eigen::AngleAxisf(o.GetAlpha(), Eigen::Vector3f::UnitZ());
                const auto b = Eigen::AngleAxisf(o.GetBeta(), Eigen::Vector3f::UnitY());
                const auto g = Eigen::AngleAxisf(o.GetGamma(), Eigen::Vector3f::UnitZ());
                const Eigen::Matrix3f actual = (a * b * g).matrix();
                AM_EXPECT(Mat3ToEigen(o.GetRotationMatrix()).isApprox(actual, 1e-4f));
            };

            // General branch cases.
            checkZYZ(Orientation(0.3f, 0.5f, 0.2f));
            checkZYZ(Orientation(1.0f, -0.4f, 0.9f));
            checkZYZ(Orientation(-0.8f, 0.9f, -1.3f));

            // Gimbal branch: pure yaw (beta == 0) — the most common listener state.
            checkZYZ(Orientation(0.7f, 0.0f, 0.0f));
            checkZYZ(Orientation(-1.9f, 0.0f, 0.0f));

            // Looking straight up: forward = +Z.
            checkZYZ(Orientation(AmVector3{ 0.0f, 0.0f, 1.0f }, AmVector3{ 0.0f, 1.0f, 0.0f }));

            // Quaternion path: 90-degree rotation about +Y decomposes to ZYZ (0, pi/2, 0).
            const Orientation pitch90(FromAxisAngle(kVector3UnitY, AM_PI32 / 2.0f));
            AM_EXPECT(std::abs(pitch90.GetBeta() - AM_PI32 / 2.0f) < 1e-4f);
            checkZYZ(pitch90);

            // beta == pi branch: rotation about +Y by pi gives R = Ry(pi), whose R(2,2)
            // is -1 -> the degenerate beta == pi sub-branch. Rz(a)Ry(pi)Rz(g) with alpha
            // folded to 0 must still reproduce the matrix (sign convention check).
            const Orientation flip(FromAxisAngle(kVector3UnitY, AM_PI32));
            AM_EXPECT(std::abs(flip.GetBeta() - AM_PI32) < 1e-4f);
            AM_EXPECT(std::abs(flip.GetGamma()) < 1e-4f);
            checkZYZ(flip);

            // beta == pi branch with nonzero folded gamma: pi-rotation about a tilted
            // XY-plane axis keeps R(2,2) == -1 but gives R(1,0) != 0, so a sign-flipped
            // gamma extraction would fail the round-trip.
            const Orientation tiltedFlip(FromAxisAngle(Normalize(AmVector3{ 0.3f, 1.0f, 0.0f }), AM_PI32));
            checkZYZ(tiltedFlip);
        }
    };

    AM_REGISTER_TEST(math_orientation, extracts_correct_zyz_angles);
} // namespace SparkyStudios::Audio::Amplitude::Tests
