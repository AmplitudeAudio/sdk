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

    // Test basic reallocation - expanding memory
    constexpr AmSize initialSize = 512;
    constexpr AmSize expandedSize = 1024;

    AmVoidPtr ptr = amMemory->Malloc(eMemoryPoolKind_Default, initialSize, __FILE__, __LINE__);
    AM_EXPECT(ptr != nullptr);
    AM_EXPECT(amMemory->SizeOf(eMemoryPoolKind_Default, ptr) >= initialSize);

    // Write some data to verify it's preserved
    auto* intData = static_cast<AmInt32*>(ptr);
    for (AmSize i = 0; i < initialSize / sizeof(AmInt32); ++i)
    {
        intData[i] = static_cast<AmInt32>(i);
    }

    // Expand the allocation
    AmVoidPtr newPtr = amMemory->Realloc(eMemoryPoolKind_Default, ptr, expandedSize, __FILE__, __LINE__);
    AM_EXPECT(newPtr != nullptr);
    AM_EXPECT(amMemory->SizeOf(eMemoryPoolKind_Default, newPtr) >= expandedSize);

    // Verify data is preserved
    auto* newIntData = static_cast<AmInt32*>(newPtr);
    for (AmSize i = 0; i < initialSize / sizeof(AmInt32); ++i)
    {
        AM_EXPECT(newIntData[i] == static_cast<AmInt32>(i));
    }

    // Test shrinking reallocation
    constexpr AmSize shrunkSize = 256;
    AmVoidPtr shrunkPtr = amMemory->Realloc(eMemoryPoolKind_Default, newPtr, shrunkSize, __FILE__, __LINE__);
    AM_EXPECT(shrunkPtr != nullptr);
    AM_EXPECT(amMemory->SizeOf(eMemoryPoolKind_Default, shrunkPtr) >= shrunkSize);

    // Verify partial data is still preserved
    auto* shrunkIntData = static_cast<AmInt32*>(shrunkPtr);
    for (AmSize i = 0; i < shrunkSize / sizeof(AmInt32); ++i)
    {
        AM_EXPECT(shrunkIntData[i] == static_cast<AmInt32>(i));
    }

    amMemory->Free(eMemoryPoolKind_Default, shrunkPtr);

    // Test reallocation with null pointer (should behave like malloc)
    AmVoidPtr nullRealloc = amMemory->Realloc(eMemoryPoolKind_Engine, nullptr, 512, __FILE__, __LINE__);
    AM_EXPECT(nullRealloc != nullptr);
    AM_EXPECT(amMemory->SizeOf(eMemoryPoolKind_Engine, nullRealloc) >= 512);
    amMemory->Free(eMemoryPoolKind_Engine, nullRealloc);

    // Test aligned reallocation
    AmVoidPtr alignedPtr = amMemory->Malign(eMemoryPoolKind_SoundData, 128, 32, __FILE__, __LINE__);
    AM_EXPECT(alignedPtr != nullptr);
    AM_EXPECT(reinterpret_cast<uintptr_t>(alignedPtr) % 32 == 0);

    // Write test data
    auto* alignedData = static_cast<AmReal32*>(alignedPtr);
    alignedData[0] = 1.0f;
    alignedData[1] = 2.0f;

    // Realign to larger size and different alignment
    AmVoidPtr realignedPtr = amMemory->Realign(eMemoryPoolKind_SoundData, alignedPtr, 256, 64, __FILE__, __LINE__);
    AM_EXPECT(realignedPtr != nullptr);
    AM_EXPECT(reinterpret_cast<uintptr_t>(realignedPtr) % 64 == 0);
    AM_EXPECT(amMemory->SizeOf(eMemoryPoolKind_SoundData, realignedPtr) >= 256);

    // Verify data preservation in aligned reallocation
    auto* realignedData = static_cast<AmReal32*>(realignedPtr);
    AM_EXPECT(realignedData[0] == 1.0f);
    AM_EXPECT(realignedData[1] == 2.0f);

    amMemory->Free(eMemoryPoolKind_SoundData, realignedPtr);

    // Test realign with null pointer (should behave like malign)
    AmVoidPtr nullRealign = amMemory->Realign(eMemoryPoolKind_Codec, nullptr, 512, 128, __FILE__, __LINE__);
    AM_EXPECT(nullRealign != nullptr);
    AM_EXPECT(reinterpret_cast<uintptr_t>(nullRealign) % 128 == 0);
    AM_EXPECT(amMemory->SizeOf(eMemoryPoolKind_Codec, nullRealign) >= 512);
    amMemory->Free(eMemoryPoolKind_Codec, nullRealign);

    MemoryManager::Deinitialize();
}
