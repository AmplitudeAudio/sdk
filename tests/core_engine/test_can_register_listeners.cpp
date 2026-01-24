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
    AM_TEST_CASE(EngineTestCase, core_engine, can_register_listeners)
    {
    public:
        void Run() override
        {
            // Test adding listeners
            Listener l1 = amEngine->AddListener(1);
            Listener l2 = amEngine->AddListener(2);

            AM_EXPECT(l1.Valid());
            AM_EXPECT(l2.Valid());

            // Test adding listener with existing ID should return same listener
            Listener l3 = amEngine->AddListener(1);
            AM_EXPECT(l3.Valid());
            AM_EXPECT_EQ(l3.GetState(), l1.GetState());

            // Test getting existing listener
            Listener l4 = amEngine->GetListener(2);
            AM_EXPECT(l4.Valid());
            AM_EXPECT_EQ(l4.GetState(), l2.GetState());

            // Test getting non-existing listener
            Listener l5 = amEngine->GetListener(3);
            AM_EXPECT_NOT(l5.Valid());

            // Test removing listeners
            amEngine->RemoveListener(1);
            amEngine->RemoveListener(&l2);
            amEngine->RemoveListener(3); // Non-existing listener should not cause issues

            AM_EXPECT_NOT(l1.Valid());
            AM_EXPECT_NOT(l2.Valid());
            AM_EXPECT_NOT(l3.Valid());
            AM_EXPECT_NOT(l4.Valid());
            AM_EXPECT_NOT(l5.Valid());

            // Test getting listener with invalid ID
            Listener l6 = amEngine->GetListener(kAmInvalidObjectId);
            AM_EXPECT_NOT(l6.Valid());

            // Test setting default listener
            Listener defaultListener = amEngine->AddListener(10);
            amEngine->SetDefaultListener(&defaultListener);
            AM_EXPECT_EQ(amEngine->GetDefaultListener().GetState(), defaultListener.GetState());

            amEngine->SetDefaultListener(nullptr);
            AM_EXPECT_NOT(amEngine->GetDefaultListener().Valid());

            // Clean up
            amEngine->RemoveListener(10);
        }
    };

    AM_REGISTER_TEST(core_engine, can_register_listeners);
} // namespace SparkyStudios::Audio::Amplitude::Tests
