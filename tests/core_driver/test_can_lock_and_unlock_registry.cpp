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

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include "MockDriver.h"
#include "ComponentTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(ComponentTestCase, core_driver, can_lock_and_unlock_registry)
    {
    public:
        void Run() override
        {
            Driver::LockRegistry();

            auto driver = std::make_shared<MockDriver>("test_locked_driver");
            auto countBefore = Driver::GetRegistry().size();

            Driver::Register(driver);
            auto countAfterRegister = Driver::GetRegistry().size();

            AM_EXPECT_EQ(countBefore, countAfterRegister);

            Driver::UnlockRegistry();

            Driver::Register(driver);
            auto countAfterUnlock = Driver::GetRegistry().size();

            AM_EXPECT_EQ(countBefore + 1, countAfterUnlock);
            AM_EXPECT(Driver::Find("test_locked_driver") != nullptr);

            Driver::Unregister(driver);
        }
    };

    AM_REGISTER_TEST(core_driver, can_lock_and_unlock_registry);
} // namespace SparkyStudios::Audio::Amplitude::Tests
