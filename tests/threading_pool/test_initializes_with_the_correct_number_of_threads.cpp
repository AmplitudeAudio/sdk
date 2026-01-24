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
    AM_TEST_CASE(SimpleTestCase, threading_pool, initializes_with_the_correct_number_of_threads)
    {
    public:
        void Run() override
        {
            Thread::Pool pool;
            pool.Init(1);

            AM_EXPECT_EQ(pool.GetThreadCount(), 1);
        }
    };

    AM_REGISTER_TEST(threading_pool, initializes_with_the_correct_number_of_threads);
} // namespace SparkyStudios::Audio::Amplitude::Tests
