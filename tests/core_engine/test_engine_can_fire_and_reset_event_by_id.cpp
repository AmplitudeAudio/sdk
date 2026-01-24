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
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(EngineTestCase, core_engine, engine_can_fire_and_reset_event_by_id)
    {
    public:
        void Run() override
        {
            // Test engine can fire and reset event by ID
            EventCanceler c1 = amEngine->Trigger(5, amEngine->AddEntity(99));
            AM_EXPECT(c1.Valid());
            AM_EXPECT(c1.GetEvent()->IsRunning());

            c1.Cancel();
            AM_EXPECT_NOT(c1.GetEvent()->IsRunning());

            // Clean up
            amEngine->RemoveEntity(99);
        }
    };

    AM_REGISTER_TEST(core_engine, engine_can_fire_and_reset_event_by_id);
} // namespace SparkyStudios::Audio::Amplitude::Tests
