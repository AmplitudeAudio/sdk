// Copyright (c) 2025-present Sparky Studios. All rights reserved.
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

#include "MemoryTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(MemoryTestCase, core_memory, memory_manager_can_initialize_and_deinitialize)
    {
    public:
        void Run() override
        {
            // Test that memory manager can initialize with default allocator
            AM_EXPECT(!MemoryManager::IsInitialized());

            MemoryManager::Initialize(nullptr);
            AM_EXPECT(MemoryManager::IsInitialized());
            AM_EXPECT(MemoryManager::GetInstance() != nullptr);

            MemoryManager::Deinitialize();
            AM_EXPECT_NOT(MemoryManager::IsInitialized());

            // Test that memory manager can initialize with custom allocator
            auto customAllocator = std::make_unique<DefaultMemoryAllocator>(2, 8 * 1024 * 1024);
            MemoryManager::Initialize(std::move(customAllocator));
            AM_EXPECT(MemoryManager::IsInitialized());
            AM_EXPECT(MemoryManager::GetInstance() != nullptr);

            MemoryManager::Deinitialize();
            AM_EXPECT_NOT(MemoryManager::IsInitialized());
        }
    };

    AM_REGISTER_TEST(core_memory, memory_manager_can_initialize_and_deinitialize);
} // namespace SparkyStudios::Audio::Amplitude::Tests
