// Copyright (c) 2026-present Sparky Studios. All rights reserved.
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

#include <chrono>
#include <future>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include "EngineTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(EngineTestCase, core_engine, wait_until_frames_returns_when_paused)
    {
    public:
        void Run() override
        {
            amEngine->Pause(true);

            // Run the wait on a helper thread: a regression must fail the test instead of hanging the suite.
            std::future<void> waiter = std::async(
                std::launch::async,
                []
                {
                    amEngine->WaitUntilFrames(5);
                });

            const bool returned = waiter.wait_for(std::chrono::seconds(1)) == std::future_status::ready;

            // Let a regressed waiter finish so the future can be joined.
            amEngine->Pause(false);
            waiter.wait();

            AM_EXPECT(returned);
        }
    };

    AM_REGISTER_TEST(core_engine, wait_until_frames_returns_when_paused);
} // namespace SparkyStudios::Audio::Amplitude::Tests
