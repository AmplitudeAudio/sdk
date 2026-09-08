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
    AM_TEST_CASE(PureUnitTestCase, math_capsule_shape, translated_and_rotated)
    {
    public:
        void Run() override
        {
            // Translated capsule: radius 1, half-height 5 -> segment endpoints local (0,0,+-4).
            CapsuleShape capsule(1.0f, 5.0f);
            capsule.SetLocation({ 10.0f, 0.0f, 0.0f });

            AM_EXPECT(capsule.Contains({ 10.0f, 0.0f, 0.0f }));
            AM_EXPECT(capsule.Contains({ 10.0f, 0.0f, 4.5f }));
            AM_EXPECT_NOT(capsule.Contains({ 0.0f, 0.0f, 0.0f }));

            // Rotated capsule: pitch 90 degrees about Y -> local +Z axis lies along world +X.
            CapsuleShape rotated(1.0f, 5.0f);
            rotated.SetOrientation(Orientation(0.0f, AM_PI32 / 2.0f, 0.0f));

            AM_EXPECT(rotated.Contains({ 4.5f, 0.0f, 0.0f }));
            AM_EXPECT_NOT(rotated.Contains({ 0.0f, 0.0f, 4.5f }));
        }
    };

    AM_REGISTER_TEST(math_capsule_shape, translated_and_rotated);
} // namespace SparkyStudios::Audio::Amplitude::Tests
