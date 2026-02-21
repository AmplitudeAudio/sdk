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

#include "DummyPoolTask.h"
#include "ComponentTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(ComponentTestCase, threading_pool, can_execute_ready_tasks)
    {
    public:
        void Run() override
        {
            Thread::Pool pool;
            pool.Init(1);

            auto task = amshared(DummyPoolTask);
            AM_EXPECT_NOT(task->IsExecuted());

            pool.AddTask(task);
            Thread::Sleep(1100); // Wait for the task to complete

            AM_EXPECT(task->IsExecuted());
            AM_EXPECT_NOT(pool.HasTasks());
        }
    };

    AM_REGISTER_TEST(threading_pool, can_execute_ready_tasks);
} // namespace SparkyStudios::Audio::Amplitude::Tests
