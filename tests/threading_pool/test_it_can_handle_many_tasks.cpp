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
#include "SimpleTestCase.h"

using namespace SparkyStudios::Audio::Amplitude;

void SimpleTestCase::Run()
{
    Thread::Pool pool;
    pool.Init(1);

    const AmThreadID threadId = Thread::GetCurrentThreadId();

    for (size_t i = 0; i < AM_MAX_THREAD_POOL_TASKS + 100; i++)
    {
        auto task = std::make_shared<DummyPoolTask>(threadId);
        ExpectFalse(task->IsExecuted(), "Task should not be executed before added to the pool");

        bool mayExecuteWorkInCallerThread = pool.GetTaskCount() >= AM_MAX_THREAD_POOL_TASKS;

        pool.AddTask(task);

        if (mayExecuteWorkInCallerThread && task->IsExecuted())
            ExpectTrue(task->GetExecutingThreadId() == threadId, "Task should be executed in the caller thread");
    }
}
