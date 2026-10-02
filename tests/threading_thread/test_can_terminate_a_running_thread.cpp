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
    AM_TEST_CASE(ComponentTestCase, threading_thread, can_terminate_a_running_thread)
    {
    public:
        void Run() override
        {
            CountingThreadWorker worker;

            AmThreadHandle handle = Thread::CreateThread(CountingThreadWorker::Run, &worker);
            AM_EXPECT_NE(nullptr, handle);

            Thread::Sleep(20);
            AM_EXPECT(worker.GetIterations() > 0);

            Thread::Terminate(handle);

            // The handle is consumed, so the caller can no longer wait on or query the thread.
            AM_EXPECT_EQ(nullptr, handle);

            // Terminate must not return until the thread is really gone, so nothing may advance after it.
            const AmInt64 afterTermination = worker.GetIterations();
            Thread::Sleep(50);
            AM_EXPECT_EQ(afterTermination, worker.GetIterations());
        }
    };

    AM_REGISTER_TEST(threading_thread, can_terminate_a_running_thread);
} // namespace SparkyStudios::Audio::Amplitude::Tests
