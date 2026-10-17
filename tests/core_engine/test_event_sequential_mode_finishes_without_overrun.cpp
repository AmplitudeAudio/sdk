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

#include <atomic>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include "EngineTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(EngineTestCase, core_engine, event_sequential_mode_finishes_without_overrun)
    {
    public:
        void Run() override
        {
            // This event has a single action and runs in Sequential mode, so the moment that
            // action finishes, the instance must stop instead of indexing past the action list.
            EventHandle event1 = amEngine->GetEventHandle("play_throw_sequential");
            AM_EXPECT(event1 != nullptr);

            EventCanceler c1 = amEngine->Trigger(event1, amEngine->AddEntity(98));
            AM_EXPECT(c1.Valid());
            AM_EXPECT(c1.GetEvent()->IsRunning());

            std::atomic<bool> isFinished{ false };
            std::atomic<bool> isAborted{ false };
            c1.GetEvent()->OnFinish(
                [&isFinished, &isAborted](bool aborted)
                {
                    isAborted = aborted;
                    isFinished = true;
                });

            AM_EXPECT(WaitUntil([&]() { return isFinished.load(); }, 600));
            AM_EXPECT_NOT(isAborted);
            AM_EXPECT_NOT(c1.GetEvent()->IsRunning());

            // Clean up
            amEngine->RemoveEntity(98);
        }
    };

    AM_REGISTER_TEST(core_engine, event_sequential_mode_finishes_without_overrun);
} // namespace SparkyStudios::Audio::Amplitude::Tests
