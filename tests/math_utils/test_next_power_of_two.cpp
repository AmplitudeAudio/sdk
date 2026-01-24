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

#include "SimpleTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(SimpleTestCase, math_utils, next_power_of_two)
    {
    public:
        void Run() override
        {
            AM_EXPECT_EQ(NextPowerOf2(1), 1);
            AM_EXPECT_EQ(NextPowerOf2(2), 2);
            AM_EXPECT_EQ(NextPowerOf2(3), 4);
            AM_EXPECT_EQ(NextPowerOf2(16), 16);
            AM_EXPECT_EQ(NextPowerOf2(250), 256);
            AM_EXPECT_EQ(NextPowerOf2(4000), 4096);
            AM_EXPECT_EQ(NextPowerOf2(1024), 1024);
            AM_EXPECT_EQ(NextPowerOf2(2024), 2048);
            AM_EXPECT_EQ(NextPowerOf2(4096), 4096);
        }
    };

    AM_REGISTER_TEST(math_utils, next_power_of_two);
} // namespace SparkyStudios::Audio::Amplitude::Tests
