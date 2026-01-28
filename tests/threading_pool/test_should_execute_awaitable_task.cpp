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

#include "AwaitableDummyPoolTask.h"
#include "SimpleTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(SimpleTestCase, threading_pool, should_execute_awaitable_task)
    {
    public:
        void Run() override
        {
            Thread::Pool pool;
            pool.Init(1);

            auto task = amshared(AwaitableDummyPoolTask);
            AM_EXPECT_NOT(task->IsExecuted());

            pool.AddTask(task);

            task->Await(10);
            AM_EXPECT_NOT(task->IsExecuted());

            task->Await();
            AM_EXPECT(task->IsExecuted());
        }
    };

    AM_REGISTER_TEST(threading_pool, should_execute_awaitable_task);
} // namespace SparkyStudios::Audio::Amplitude::Tests
