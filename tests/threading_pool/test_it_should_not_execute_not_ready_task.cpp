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

#include "NeverReadyPoolTask.h"
#include "SimpleTestCase.h"

using namespace SparkyStudios::Audio::Amplitude;

void SimpleTestCase::Run()
{
    Thread::Pool pool;
    pool.Init(1);

    auto task = AmSharedPtr<NeverReadyPoolTask>::Make();
    ExpectFalse(task->IsExecuted(), "Task should not be executed yet");

    pool.AddTask(task);

    Thread::Sleep(50); // Wait for the task to complete
    ExpectFalse(task->IsExecuted(), "Task should not be executed");

    ExpectTrue(pool.HasTasks(), "Pool should still have the task");
    ExpectEqual(pool.GetTaskCount(), 1);
}
