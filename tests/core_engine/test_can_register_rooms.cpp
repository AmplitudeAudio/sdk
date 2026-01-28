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
    AM_TEST_CASE(EngineTestCase, core_engine, can_register_rooms)
    {
    public:
        void Run() override
        {
            // Test adding rooms
            Room r1 = amEngine->AddRoom(1);
            Room r2 = amEngine->AddRoom(2);

            AM_EXPECT(r1.Valid());
            AM_EXPECT(r2.Valid());

            // Test adding room with existing ID should return same room
            Room r3 = amEngine->AddRoom(1);
            AM_EXPECT(r3.Valid());
            AM_EXPECT_EQ(r3.GetState(), r1.GetState());

            // Test getting existing room
            Room r4 = amEngine->GetRoom(2);
            AM_EXPECT(r4.Valid());
            AM_EXPECT_EQ(r4.GetState(), r2.GetState());

            // Test getting non-existing room
            Room r5 = amEngine->GetRoom(3);
            AM_EXPECT_NOT(r5.Valid());

            // Test removing rooms
            amEngine->RemoveRoom(1);
            amEngine->RemoveRoom(&r2);
            amEngine->RemoveRoom(3); // Non-existing room should not cause issues

            AM_EXPECT_NOT(r1.Valid());
            AM_EXPECT_NOT(r2.Valid());
            AM_EXPECT_NOT(r3.Valid());
            AM_EXPECT_NOT(r4.Valid());
            AM_EXPECT_NOT(r5.Valid());

            // Test getting room with invalid ID
            Room r6 = amEngine->GetRoom(kAmInvalidObjectId);
            AM_EXPECT_NOT(r6.Valid());
        }
    };

    AM_REGISTER_TEST(core_engine, can_register_rooms);
} // namespace SparkyStudios::Audio::Amplitude::Tests
