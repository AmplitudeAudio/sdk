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
    AM_TEST_CASE(PureUnitTestCase, math_orientation, constructs_from_forward_up_vectors)
    {
    public:
        void Run() override
        {
            constexpr AmVector3 forward = kVector3UnitY;
            constexpr AmVector3 up = kVector3UnitZ;

            const Orientation orientation(forward, up);

            // it should normalize and store the vectors
        {
            const auto storedForward = orientation.GetForward();
            const auto storedUp = orientation.GetUp();

            // Vectors should be normalized
            AM_EXPECT(std::abs(Length(storedForward) - 1.0f) < kEpsilon);
            AM_EXPECT(std::abs(Length(storedUp) - 1.0f) < kEpsilon);

            // Should match normalized input vectors
            AM_EXPECT_EQ(storedForward, Normalize(forward));
            AM_EXPECT_EQ(storedUp, Normalize(up));
            }

            // it should compute corresponding Euler angles
        {
            // ZYX angles should be computed from the vectors
            const auto yaw = orientation.GetYaw();
            const auto pitch = orientation.GetPitch();
            const auto roll = orientation.GetRoll();

            // Reconstructed vectors from angles should match the original
            const Orientation reconstructed(yaw, pitch, roll);
            const auto reconstructedForward = reconstructed.GetForward();
            const auto reconstructedUp = reconstructed.GetUp();

            AM_EXPECT_EQ(reconstructedForward, orientation.GetForward());
            AM_EXPECT_EQ(reconstructedUp, orientation.GetUp());
            }
        }
    };

    AM_REGISTER_TEST(math_orientation, constructs_from_forward_up_vectors);
} // namespace SparkyStudios::Audio::Amplitude::Tests
