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
    AM_TEST_CASE(ComponentTestCase, core_driver, can_set_default_driver)
    {
    public:
        void Run() override
        {
            auto driver1 = std::make_shared<MockDriver>("test_default_driver_1");
            auto driver2 = std::make_shared<MockDriver>("test_default_driver_2");

            Driver::Register(driver1);
            Driver::Register(driver2);

            Driver::SetDefault("test_default_driver_1");

            auto defaultDriver = Driver::Default();
            AM_EXPECT(defaultDriver != nullptr);
            AM_EXPECT_EQ("test_default_driver_1", defaultDriver->GetName());

            Driver::Unregister(driver1);
            Driver::Unregister(driver2);
        }
    };

    AM_REGISTER_TEST(core_driver, can_set_default_driver);
} // namespace SparkyStudios::Audio::Amplitude::Tests
