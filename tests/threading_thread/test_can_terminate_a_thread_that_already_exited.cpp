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
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    namespace
    {
        void ReturningThreadWorker(AmVoidPtr param)
        {
            AM_UNUSED(param);
        }
    } // namespace

    AM_TEST_CASE(ComponentTestCase, threading_thread, can_terminate_a_thread_that_already_exited)
    {
    public:
        void Run() override
        {
            AmThreadHandle handle = Thread::CreateThread(ReturningThreadWorker, nullptr);
            AM_EXPECT_NE(nullptr, handle);

            // Let the thread run to completion on its own, so the kill lands on an already dead thread.
            Thread::Sleep(50);

            // On Windows this is the path where TerminateThread fails because the thread is already
            // gone. Terminate must still consume the handle and wait for a thread that is already dead.
            Thread::Terminate(handle);
            AM_EXPECT_EQ(nullptr, handle);
        }
    };

    AM_REGISTER_TEST(threading_thread, can_terminate_a_thread_that_already_exited);
} // namespace SparkyStudios::Audio::Amplitude::Tests
