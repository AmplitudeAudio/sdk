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

#include "ComponentTestCase.h"
#include "CountingThreadWorker.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(ComponentTestCase, threading_thread, can_release_a_thread_without_waiting_for_it)
    {
    public:
        void Run() override
        {
            CountingThreadWorker worker;

            AmThreadHandle handle = Thread::CreateThread(CountingThreadWorker::Run, &worker);

            Thread::Sleep(20);
            AM_EXPECT(worker.GetIterations() > 0);

            const auto beforeRelease = Thread::GetTimeMillis();
            Thread::Release(handle);
            const auto releaseDuration = Thread::GetTimeMillis() - beforeRelease;

            AM_EXPECT_EQ(nullptr, handle);

            // Release detaches, it does not wait: the worker would run for as long as it keeps looping.
            AM_EXPECT(releaseDuration < 100);

            // Detaching only gives up the right to observe the thread. The thread itself keeps running
            // and releasing the handle must not have stopped it.
            const AmInt64 atRelease = worker.GetIterations();
            Thread::Sleep(50);
            AM_EXPECT(worker.GetIterations() > atRelease);

            // Nothing can join a detached thread, so the worker is the only one that can stop it. It has
            // to be fully stopped before the memory manager is torn down in TearDown.
            worker.Stop();
            Thread::Sleep(50);
        }
    };

    AM_REGISTER_TEST(threading_thread, can_release_a_thread_without_waiting_for_it);
} // namespace SparkyStudios::Audio::Amplitude::Tests
