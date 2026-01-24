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
    AM_TEST_CASE(SimpleTestCase, math_orientation, is_consistent_between_representations)
    {
    public:
        void Run() override
        {
            constexpr AmReal32 yaw = AM_DegToRad * 60.0f;
            constexpr AmReal32 pitch = AM_DegToRad * 20.0f;
            constexpr AmReal32 roll = AM_DegToRad * 10.0f;

            const Orientation originalOrientation(yaw, pitch, roll);

        {
            const Orientation reconstructedFromVectors(originalOrientation.GetForward(), originalOrientation.GetUp());

            AM_EXPECT_EQ(originalOrientation.GetForward(), reconstructedFromVectors.GetForward());
            AM_EXPECT_EQ(originalOrientation.GetUp(), reconstructedFromVectors.GetUp());
            }

        {
            const Orientation reconstructedFromQuaternion(originalOrientation.GetQuaternion());

            AM_EXPECT(Length(Sub(originalOrientation.GetForward(), reconstructedFromQuaternion.GetForward())) < kEpsilon);
            AM_EXPECT(Length(Sub(originalOrientation.GetUp(), reconstructedFromQuaternion.GetUp())) < kEpsilon);
            }
        }
    };

    AM_REGISTER_TEST(math_orientation, is_consistent_between_representations);
} // namespace SparkyStudios::Audio::Amplitude::Tests
