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

#include "PureUnitTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(PureUnitTestCase, math_box_shape, translated_and_rotated)
    {
    public:
        void Run() override
        {
            // Translated box (half extents 1, 2, 3) at (10, 0, 0).
            BoxShape box(1.0f, 2.0f, 3.0f);
            box.SetLocation({ 10.0f, 0.0f, 0.0f });

            AM_EXPECT(box.Contains({ 10.0f, 0.0f, 0.0f }));
            AM_EXPECT(box.Contains({ 10.5f, 2.5f, 1.5f }));
            AM_EXPECT_NOT(box.Contains({ 0.0f, 0.0f, 0.0f }));
            AM_EXPECT_NOT(box.Contains({ 12.0f, 0.0f, 0.0f }));

            const auto corners = box.GetCorners();
            AM_EXPECT_EQ(corners[0], (AmVector3{ 9.0f, -3.0f, -2.0f }));
            AM_EXPECT_EQ(corners[4], (AmVector3{ 11.0f, 3.0f, 2.0f }));
            AM_EXPECT_EQ(corners[6], (AmVector3{ 11.0f, 3.0f, -2.0f }));
            AM_EXPECT_EQ(corners[7], (AmVector3{ 11.0f, -3.0f, 2.0f }));

            AM_EXPECT_EQ(box.GetClosestPoint({ 20.0f, 0.0f, 0.0f }), (AmVector3{ 11.0f, 0.0f, 0.0f }));
            AM_EXPECT_EQ(box.GetClosestPoint({ 10.0f, 0.0f, 0.0f }), (AmVector3{ 10.0f, 0.0f, 0.0f }));

            // Box rotated 90 degrees about Z: local +X (width) points along world +Y.
            BoxShape rotated(1.0f, 2.0f, 3.0f);
            rotated.SetOrientation(Orientation(AM_PI32 / 2.0f, 0.0f, 0.0f));

            AM_EXPECT(rotated.Contains({ 0.0f, 0.5f, 0.0f }));
            AM_EXPECT_NOT(rotated.Contains({ 0.0f, 1.5f, 0.0f }));
            AM_EXPECT(rotated.Contains({ 0.5f, 0.0f, 0.0f }));
            AM_EXPECT_NOT(rotated.Contains({ 3.5f, 0.0f, 0.0f }));

            const auto rotatedCorners = rotated.GetCorners();
            // Floating point imprecision in sin/cos(PI/2) prevents exact comparison
            ExpectFloatNear(3.0f, rotatedCorners[0].x);
            ExpectFloatNear(-1.0f, rotatedCorners[0].y);
            ExpectFloatNear(-2.0f, rotatedCorners[0].z);

            auto closestRotated = rotated.GetClosestPoint({ 0.0f, 5.0f, 0.0f });
            ExpectFloatNear(0.0f, closestRotated.x);
            ExpectFloatNear(1.0f, closestRotated.y);
            ExpectFloatNear(0.0f, closestRotated.z);
        }
    };

    AM_REGISTER_TEST(math_box_shape, translated_and_rotated);
} // namespace SparkyStudios::Audio::Amplitude::Tests
