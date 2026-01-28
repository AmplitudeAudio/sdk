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
    AM_TEST_CASE(EngineTestCase, core_engine, cannot_load_invalid_event_handles)
    {
    public:
        void Run() override
        {
            // Test loading invalid event handles
            EventHandle invalidEvent1 = amEngine->GetEventHandle("invalid_event");
            AM_EXPECT(invalidEvent1 == nullptr);

            EventHandle invalidEvent2 = amEngine->GetEventHandle(99999999);
            AM_EXPECT(invalidEvent2 == nullptr);

            // Create an entity for testing event triggering
            const Entity& e = amEngine->AddEntity(99);

            // Test triggering events with invalid event IDs (should not crash)
            EventCanceler canceler = amEngine->Trigger(99999999, e);
            AM_EXPECT_NOT(canceler.Valid());

            // Test triggering events with invalid event names (should not crash)
            EventCanceler canceler2 = amEngine->Trigger("invalid_event", e);
            AM_EXPECT_NOT(canceler2.Valid());

            // Test triggering events with invalid event handles (should not crash)
            EventCanceler canceler3 = amEngine->Trigger(invalidEvent2, e);
            AM_EXPECT_NOT(canceler3.Valid());

            // Clean up
            amEngine->RemoveEntity(99);
        }
    };

    AM_REGISTER_TEST(core_engine, cannot_load_invalid_event_handles);
} // namespace SparkyStudios::Audio::Amplitude::Tests
