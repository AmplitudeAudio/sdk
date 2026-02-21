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

#include "PureUnitTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(PureUnitTestCase, math_orientation, zero_orientation)
    {
    public:
        void Run() override
        {
            const auto zero = Orientation::Zero();

            // Test that Zero() creates an orientation with zero angles
            AM_EXPECT_EQ(zero.GetYaw(), 0.0f);
            AM_EXPECT_EQ(zero.GetPitch(), 0.0f);
            AM_EXPECT_EQ(zero.GetRoll(), 0.0f);

            // Test forward and up vectors for zero orientations
            AM_EXPECT_EQ(zero.GetForward(), kVector3UnitY);
            AM_EXPECT_EQ(zero.GetUp(), kVector3UnitZ);
        }
    };

    AM_REGISTER_TEST(math_orientation, zero_orientation);
} // namespace SparkyStudios::Audio::Amplitude::Tests
