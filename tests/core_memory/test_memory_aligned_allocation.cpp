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
    AM_TEST_CASE(MemoryTestCase, core_memory, memory_aligned_allocation)
    {
    public:
        void Run() override
        {
            // Initialize memory manager
            MemoryManager::Initialize(nullptr);

            // Test aligned allocation with different alignments
            constexpr AmSize allocSize = 1024;

            // Test 16-byte alignment
            AmVoidPtr ptr1 = amMemory->Malign(eMemoryPoolKind_Default, allocSize, 16, __FILE__, __LINE__);
            AM_EXPECT(ptr1 != nullptr);
            AM_EXPECT(reinterpret_cast<uintptr_t>(ptr1) % 16 == 0);
            AM_EXPECT(amMemory->SizeOf(eMemoryPoolKind_Default, ptr1) >= allocSize);

            // Test 32-byte alignment
            AmVoidPtr ptr2 = amMemory->Malign(eMemoryPoolKind_Engine, allocSize, 32, __FILE__, __LINE__);
            AM_EXPECT(ptr2 != nullptr);
            AM_EXPECT(reinterpret_cast<uintptr_t>(ptr2) % 32 == 0);
            AM_EXPECT(amMemory->SizeOf(eMemoryPoolKind_Engine, ptr2) >= allocSize);

            // Test 64-byte alignment
            AmVoidPtr ptr3 = amMemory->Malign(eMemoryPoolKind_SoundData, allocSize, 64, __FILE__, __LINE__);
            AM_EXPECT(ptr3 != nullptr);
            AM_EXPECT(reinterpret_cast<uintptr_t>(ptr3) % 64 == 0);
            AM_EXPECT(amMemory->SizeOf(eMemoryPoolKind_SoundData, ptr3) >= allocSize);

            // Test 128-byte alignment
            AmVoidPtr ptr4 = amMemory->Malign(eMemoryPoolKind_Filtering, allocSize, 128, __FILE__, __LINE__);
            AM_EXPECT(ptr4 != nullptr);
            AM_EXPECT(reinterpret_cast<uintptr_t>(ptr4) % 128 == 0);
            AM_EXPECT(amMemory->SizeOf(eMemoryPoolKind_Filtering, ptr4) >= allocSize);

            // Test writing to aligned memory
            auto* floatPtr = static_cast<AmReal32*>(ptr1);
            *floatPtr = 3.14159f;
            AM_EXPECT(*floatPtr == 3.14159f);

            // Test SIMD-friendly alignment (256-byte for AVX)
            AmVoidPtr ptr5 = amMemory->Malign(eMemoryPoolKind_Amplimix, allocSize, 256, __FILE__, __LINE__);
            AM_EXPECT(ptr5 != nullptr);
            AM_EXPECT(reinterpret_cast<uintptr_t>(ptr5) % 256 == 0);

            // Test power-of-two alignments
            for (AmUInt32 alignment = 1; alignment <= 1024; alignment <<= 1)
            {
                AmVoidPtr ptr = amMemory->Malign(eMemoryPoolKind_Default, 64, alignment, __FILE__, __LINE__);
                AM_EXPECT(ptr != nullptr);
                AM_EXPECT(reinterpret_cast<uintptr_t>(ptr) % alignment == 0);
                amMemory->Free(eMemoryPoolKind_Default, ptr);
            }

            // Clean up
            amMemory->Free(eMemoryPoolKind_Default, ptr1);
            amMemory->Free(eMemoryPoolKind_Engine, ptr2);
            amMemory->Free(eMemoryPoolKind_SoundData, ptr3);
            amMemory->Free(eMemoryPoolKind_Filtering, ptr4);
            amMemory->Free(eMemoryPoolKind_Amplimix, ptr5);

            MemoryManager::Deinitialize();
        }
    };

    AM_REGISTER_TEST(core_memory, memory_aligned_allocation);
} // namespace SparkyStudios::Audio::Amplitude::Tests
