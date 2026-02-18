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

#pragma once

#include "TestCase.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    /**
     * @brief Test case base class for MiniAudio driver tests.
     *
     * This test case sets up the environment for testing the MiniAudio driver
     * without the full engine initialization. It registers default extensions
     * and makes the MiniAudio driver available for testing.
     */
    class MiniAudioDriverTestCase : public TestCase
    {
    public:
        void SetUp() override
        {
            MemoryManager::Initialize();
            Engine::RegisterDefaultExtensions();
        }

        void TearDown() override
        {
            Engine::UnregisterDefaultExtensions();
            amEngine->DestroyInstance();
            MemoryManager::Deinitialize();
        }

    protected:
        /**
         * @brief Gets the MiniAudio driver instance.
         *
         * @return The MiniAudio driver, or nullptr if not registered.
         */
        [[nodiscard]] std::shared_ptr<Driver> GetMiniAudioDriver() const
        {
            return Driver::Find("miniaudio");
        }
    };
} // namespace SparkyStudios::Audio::Amplitude::Tests
