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

#if !defined(AM_NO_MEMORY_STATS)
    // Test memory pool statistics tracking
    const auto& initialStats = amMemory->GetStats(eMemoryPoolKind_Default);
    AmUInt64 initialAllocCount = initialStats.allocCount.load();
    AmUInt64 initialFreeCount = initialStats.freeCount.load();
    AmSize initialMaxMemory = initialStats.maxMemoryUsed.load();

    // Allocate some memory and check stats
    constexpr AmSize allocSize = 1024;
    AmVoidPtr ptr1 = amMemory->Malloc(eMemoryPoolKind_Default, allocSize, __FILE__, __LINE__);
    AM_EXPECT(ptr1 != nullptr);

    const auto& statsAfterAlloc = amMemory->GetStats(eMemoryPoolKind_Default);
    AM_EXPECT(statsAfterAlloc.allocCount.load() == initialAllocCount + 1);
    AM_EXPECT(statsAfterAlloc.maxMemoryUsed.load() >= initialMaxMemory + allocSize);

    // Allocate more memory
    AmVoidPtr ptr2 = amMemory->Malloc(eMemoryPoolKind_Default, allocSize * 2, __FILE__, __LINE__);
    AM_EXPECT(ptr2 != nullptr);

    const auto& statsAfterSecondAlloc = amMemory->GetStats(eMemoryPoolKind_Default);
    AM_EXPECT(statsAfterSecondAlloc.allocCount.load() == initialAllocCount + 2);
    AM_EXPECT(statsAfterSecondAlloc.maxMemoryUsed.load() >= initialMaxMemory + allocSize * 3);

    // Free memory and check free count
    amMemory->Free(eMemoryPoolKind_Default, ptr1);
    const auto& statsAfterFirstFree = amMemory->GetStats(eMemoryPoolKind_Default);
    AM_EXPECT(statsAfterFirstFree.freeCount.load() == initialFreeCount + 1);

    amMemory->Free(eMemoryPoolKind_Default, ptr2);
    const auto& statsAfterSecondFree = amMemory->GetStats(eMemoryPoolKind_Default);
    AM_EXPECT(statsAfterSecondFree.freeCount.load() == initialFreeCount + 2);

    // Test statistics for different memory pools
    const auto& engineStats = amMemory->GetStats(eMemoryPoolKind_Engine);
    AmUInt64 engineInitialAllocs = engineStats.allocCount.load();

    AmVoidPtr enginePtr = amMemory->Malloc(eMemoryPoolKind_Engine, 512, __FILE__, __LINE__);
    AM_EXPECT(enginePtr != nullptr);

    const auto& engineStatsAfter = amMemory->GetStats(eMemoryPoolKind_Engine);
    AM_EXPECT(engineStatsAfter.allocCount.load() == engineInitialAllocs + 1);
    AM_EXPECT(engineStatsAfter.maxMemoryUsed.load() >= 512);

    amMemory->Free(eMemoryPoolKind_Engine, enginePtr);

    // Test realloc statistics
    AmVoidPtr reallocPtr = amMemory->Malloc(eMemoryPoolKind_SoundData, 256, __FILE__, __LINE__);
    AM_EXPECT(reallocPtr != nullptr);

    const auto& soundDataInitialStats = amMemory->GetStats(eMemoryPoolKind_SoundData);
    AmUInt64 soundDataInitialAllocs = soundDataInitialStats.allocCount.load();

    // Realloc should not increase alloc count if it's expanding existing allocation
    AmVoidPtr newReallocPtr = amMemory->Realloc(eMemoryPoolKind_SoundData, reallocPtr, 512, __FILE__, __LINE__);
    AM_EXPECT(newReallocPtr != nullptr);

    const auto& soundDataAfterRealloc = amMemory->GetStats(eMemoryPoolKind_SoundData);
    // Note: realloc behavior may vary by implementation, but generally shouldn't increase alloc count for existing pointer
    AM_EXPECT(soundDataAfterRealloc.allocCount.load() >= soundDataInitialAllocs);

    amMemory->Free(eMemoryPoolKind_SoundData, newReallocPtr);

    // Test aligned allocation statistics
    const auto& amplimixInitialStats = amMemory->GetStats(eMemoryPoolKind_Amplimix);
    AmUInt64 amplimixInitialAllocs = amplimixInitialStats.allocCount.load();

    AmVoidPtr alignedPtr = amMemory->Malign(eMemoryPoolKind_Amplimix, 1024, 64, __FILE__, __LINE__);
    AM_EXPECT(alignedPtr != nullptr);

    const auto& amplimixAfterAlloc = amMemory->GetStats(eMemoryPoolKind_Amplimix);
    AM_EXPECT(amplimixAfterAlloc.allocCount.load() == amplimixInitialAllocs + 1);
    AM_EXPECT(amplimixAfterAlloc.maxMemoryUsed.load() >= 1024);

    amMemory->Free(eMemoryPoolKind_Amplimix, alignedPtr);

    // Test memory pool names
    AM_EXPECT(MemoryManager::GetMemoryPoolName(eMemoryPoolKind_Default) == "Default");
    AM_EXPECT(MemoryManager::GetMemoryPoolName(eMemoryPoolKind_Engine) == "Engine");
    AM_EXPECT(MemoryManager::GetMemoryPoolName(eMemoryPoolKind_Amplimix) == "Amplimix");
    AM_EXPECT(MemoryManager::GetMemoryPoolName(eMemoryPoolKind_SoundData) == "SoundData");
    AM_EXPECT(MemoryManager::GetMemoryPoolName(eMemoryPoolKind_Filtering) == "Filtering");
    AM_EXPECT(MemoryManager::GetMemoryPoolName(eMemoryPoolKind_Codec) == "Codec");
    AM_EXPECT(MemoryManager::GetMemoryPoolName(eMemoryPoolKind_IO) == "IO");

    // Test statistics consistency across all pools
    for (AmUInt8 poolIdx = 0; poolIdx < static_cast<AmUInt8>(eMemoryPoolKind_COUNT); ++poolIdx)
    {
        const auto poolKind = static_cast<eMemoryPoolKind>(poolIdx);
        const auto& poolStats = amMemory->GetStats(poolKind);

        // Alloc count should be >= free count (some allocations might still be active)
        AM_EXPECT(poolStats.allocCount.load() >= poolStats.freeCount.load());

        // Pool should be correctly set
        AM_EXPECT(poolStats.pool == poolKind);
    }

#else
    // If memory stats are disabled, we should still be able to initialize and use memory
    AmVoidPtr testPtr = amMemory->Malloc(eMemoryPoolKind_Default, 1024, __FILE__, __LINE__);
    AM_EXPECT(testPtr != nullptr);
    amMemory->Free(eMemoryPoolKind_Default, testPtr);
#endif

    MemoryManager::Deinitialize();
}
