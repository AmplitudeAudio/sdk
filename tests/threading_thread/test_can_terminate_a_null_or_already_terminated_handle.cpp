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
    AM_TEST_CASE(ComponentTestCase, threading_thread, can_terminate_a_null_or_already_terminated_handle)
    {
    public:
        void Run() override
        {
            AmThreadHandle emptyHandle = nullptr;
            Thread::Terminate(emptyHandle);
            AM_EXPECT_EQ(nullptr, emptyHandle);

            CountingThreadWorker worker;

            AmThreadHandle handle = Thread::CreateThread(CountingThreadWorker::Run, &worker);
            Thread::Sleep(20);

            Thread::Terminate(handle);
            AM_EXPECT_EQ(nullptr, handle);

            // The handle was consumed and nulled, so terminating the same variable again is a no-op
            // rather than a use-after-free on the freed wrapper.
            Thread::Terminate(handle);
            AM_EXPECT_EQ(nullptr, handle);
        }
    };

    AM_REGISTER_TEST(threading_thread, can_terminate_a_null_or_already_terminated_handle);
} // namespace SparkyStudios::Audio::Amplitude::Tests
