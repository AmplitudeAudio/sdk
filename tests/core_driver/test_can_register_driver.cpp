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
#include "SimpleTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(SimpleTestCase, core_driver, can_register_driver)
    {
    public:
        void Run() override
        {
            auto driver = std::make_shared<MockDriver>("test_register_driver");
            auto initialCount = Driver::GetRegistry().size();

            Driver::Register(driver);

            AM_EXPECT_EQ(initialCount + 1, Driver::GetRegistry().size());
            AM_EXPECT(Driver::Find("test_register_driver") != nullptr);

            Driver::Unregister(driver);
        }
    };

    AM_REGISTER_TEST(core_driver, can_register_driver);
} // namespace SparkyStudios::Audio::Amplitude::Tests
