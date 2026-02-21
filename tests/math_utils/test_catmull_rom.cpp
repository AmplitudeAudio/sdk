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
    AM_TEST_CASE(PureUnitTestCase, math_utils, catmull_rom)
    {
    public:
        void Run() override
        {
            constexpr AmReal32 p1 = 0.0f;
            constexpr AmReal32 p2 = 1.0f;
            constexpr AmReal32 p3 = 2.0f;
            constexpr AmReal32 p4 = 3.0f;

            AM_EXPECT_EQ(CatmullRom(0.0f, p1, p2, p3, p4), 1.0f);
            AM_EXPECT_EQ(CatmullRom(1.0f, p1, p2, p3, p4), 2.0f);
            AM_EXPECT_EQ(CatmullRom(0.5f, p1, p2, p3, p4), 1.5f);
        }
    };

    AM_REGISTER_TEST(math_utils, catmull_rom);
} // namespace SparkyStudios::Audio::Amplitude::Tests
