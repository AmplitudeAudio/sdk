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

using namespace SparkyStudios::Audio::Amplitude;

void MemoryTestCase::Run()
{
    // Initialize memory manager
    MemoryManager::Initialize(nullptr);

    // Test basic allocation and deallocation
    constexpr AmSize allocSize = 1024;

    // Test allocation from default pool
    AmVoidPtr ptr1 = amMemory->Malloc(eMemoryPoolKind_Default, allocSize, __FILE__, __LINE__);
    AM_EXPECT(ptr1 != nullptr);
    AM_EXPECT(amMemory->SizeOf(eMemoryPoolKind_Default, ptr1) >= allocSize);

    // Test allocation from engine pool
    AmVoidPtr ptr2 = amMemory->Malloc(eMemoryPoolKind_Engine, allocSize, __FILE__, __LINE__);
    AM_EXPECT(ptr2 != nullptr);
    AM_EXPECT(amMemory->SizeOf(eMemoryPoolKind_Engine, ptr2) >= allocSize);

    // Test allocation from different pools
    AmVoidPtr ptr3 = amMemory->Malloc(eMemoryPoolKind_SoundData, allocSize * 2, __FILE__, __LINE__);
    AM_EXPECT(ptr3 != nullptr);
    AM_EXPECT(amMemory->SizeOf(eMemoryPoolKind_SoundData, ptr3) >= allocSize * 2);

    // Verify pointers are different
    AM_EXPECT(ptr1 != ptr2);
    AM_EXPECT(ptr2 != ptr3);
    AM_EXPECT(ptr1 != ptr3);

    // Test writing to allocated memory
    auto* intPtr = static_cast<AmInt32*>(ptr1);
    *intPtr = 42;
    AM_EXPECT(*intPtr == 42);

    // Test deallocation
    amMemory->Free(eMemoryPoolKind_Default, ptr1);
    amMemory->Free(eMemoryPoolKind_Engine, ptr2);
    amMemory->Free(eMemoryPoolKind_SoundData, ptr3);

    // Test zero-size allocation
    AmVoidPtr ptr4 = amMemory->Malloc(eMemoryPoolKind_Default, 0, __FILE__, __LINE__);
    AM_EXPECT(ptr4 != nullptr); // Many allocators return a valid pointer for zero-size allocations
    amMemory->Free(eMemoryPoolKind_Default, ptr4);

    MemoryManager::Deinitialize();
}
