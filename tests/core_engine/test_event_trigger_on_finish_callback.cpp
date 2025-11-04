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

#include "EngineTestCase.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    void EngineTestCase::Run()
    {
        EventHandle event1 = amEngine->GetEventHandle("play_throw");
        AM_EXPECT(event1 != nullptr);

        {
            // Test event can fire and reset
            EventCanceler c1 = amEngine->Trigger(event1, amEngine->AddEntity(99));
            AM_EXPECT(c1.Valid());
            AM_EXPECT(c1.GetEvent()->IsRunning());

            bool isFinished = false;
            bool isAborted = false;
            c1.GetEvent()->OnFinish([&isFinished, &isAborted](bool aborted) {
                isFinished = true;
                isAborted = aborted;
            });

            AM_EXPECT_NOT(isFinished);
            AM_EXPECT_NOT(isAborted);
            Thread::Sleep(10000);
            AM_EXPECT(isFinished);
            AM_EXPECT_NOT(isAborted);

            AM_EXPECT_NOT(c1.GetEvent()->IsRunning());

            // Clean up
            amEngine->RemoveEntity(99);
        }

        {
            // Test event can fire and reset
            EventCanceler c1 = amEngine->Trigger(event1, amEngine->AddEntity(97));
            AM_EXPECT(c1.Valid());
            AM_EXPECT(c1.GetEvent()->IsRunning());

            bool isFinished = false;
            bool isAborted = false;
            c1.GetEvent()->OnFinish([&isFinished, &isAborted](bool aborted) {
                isFinished = true;
                isAborted = aborted;
            });

            AM_EXPECT_NOT(isFinished);
            AM_EXPECT_NOT(isAborted);
            c1.Cancel();
            AM_EXPECT(isFinished);
            AM_EXPECT(isAborted);

            AM_EXPECT_NOT(c1.GetEvent()->IsRunning());

            // Clean up
            amEngine->RemoveEntity(97);
        }
    }
} // namespace SparkyStudios::Audio::Amplitude::Tests
