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
    AM_TEST_CASE(MemoryTestCase, core_memory, memory_pool_isolation)
    {
    public:
        void Run() override
        {
            // Initialize memory manager
            MemoryManager::Initialize(nullptr);

            // Test that different memory pools are isolated from each other
            constexpr AmSize allocSize = 1024;

            // Record initial memory usage for each pool
            AmSize initialEngine = amMemory->TotalReservedMemorySize(eMemoryPoolKind_Engine);
            AmSize initialAmplimix = amMemory->TotalReservedMemorySize(eMemoryPoolKind_Amplimix);
            AmSize initialSoundData = amMemory->TotalReservedMemorySize(eMemoryPoolKind_SoundData);
            AmSize initialFiltering = amMemory->TotalReservedMemorySize(eMemoryPoolKind_Filtering);
            AmSize initialCodec = amMemory->TotalReservedMemorySize(eMemoryPoolKind_Codec);
            AmSize initialIO = amMemory->TotalReservedMemorySize(eMemoryPoolKind_IO);
            AmSize initialDefault = amMemory->TotalReservedMemorySize(eMemoryPoolKind_Default);

            // Allocate memory from Engine pool
            AmVoidPtr enginePtr = amMemory->Malloc(eMemoryPoolKind_Engine, allocSize, __FILE__, __LINE__);
            AM_EXPECT(enginePtr != nullptr);

            // Verify only Engine pool memory usage increased
            AM_EXPECT(amMemory->TotalReservedMemorySize(eMemoryPoolKind_Engine) > initialEngine);
            AM_EXPECT(amMemory->TotalReservedMemorySize(eMemoryPoolKind_Amplimix) == initialAmplimix);
            AM_EXPECT(amMemory->TotalReservedMemorySize(eMemoryPoolKind_SoundData) == initialSoundData);
            AM_EXPECT(amMemory->TotalReservedMemorySize(eMemoryPoolKind_Filtering) == initialFiltering);
            AM_EXPECT(amMemory->TotalReservedMemorySize(eMemoryPoolKind_Codec) == initialCodec);
            AM_EXPECT(amMemory->TotalReservedMemorySize(eMemoryPoolKind_IO) == initialIO);
            AM_EXPECT(amMemory->TotalReservedMemorySize(eMemoryPoolKind_Default) == initialDefault);

            // Allocate from different pools and verify isolation
            AmVoidPtr amplimixPtr = amMemory->Malloc(eMemoryPoolKind_Amplimix, allocSize * 2, __FILE__, __LINE__);
            AM_EXPECT(amplimixPtr != nullptr);
            AM_EXPECT(amMemory->TotalReservedMemorySize(eMemoryPoolKind_Amplimix) > initialAmplimix);

            AmVoidPtr soundDataPtr = amMemory->Malign(eMemoryPoolKind_SoundData, allocSize, 64, __FILE__, __LINE__);
            AM_EXPECT(soundDataPtr != nullptr);
            AM_EXPECT(amMemory->TotalReservedMemorySize(eMemoryPoolKind_SoundData) > initialSoundData);

            AmVoidPtr filteringPtr = amMemory->Malloc(eMemoryPoolKind_Filtering, allocSize / 2, __FILE__, __LINE__);
            AM_EXPECT(filteringPtr != nullptr);
            AM_EXPECT(amMemory->TotalReservedMemorySize(eMemoryPoolKind_Filtering) > initialFiltering);

            // Verify all pointers are different
            AM_EXPECT(enginePtr != amplimixPtr);
            AM_EXPECT(enginePtr != soundDataPtr);
            AM_EXPECT(enginePtr != filteringPtr);
            AM_EXPECT(amplimixPtr != soundDataPtr);
            AM_EXPECT(amplimixPtr != filteringPtr);
            AM_EXPECT(soundDataPtr != filteringPtr);

            // Test that we can write to memory from different pools without interference
            auto* engineData = static_cast<AmInt32*>(enginePtr);
            auto* amplimixData = static_cast<AmReal32*>(amplimixPtr);
            auto* soundData = static_cast<AmInt64*>(soundDataPtr);
            auto* filterData = static_cast<AmReal64*>(filteringPtr);

            // Write distinct patterns to each pool
            *engineData = 0xDEADBEEF;
            *amplimixData = 3.14159f;
            *soundData = 0x123456789ABCDEF0LL;
            *filterData = 2.71828;

            // Verify data integrity across pools
            AM_EXPECT(*engineData == static_cast<AmInt32>(0xDEADBEEF));
            AM_EXPECT(*amplimixData == 3.14159f);
            AM_EXPECT(*soundData == 0x123456789ABCDEF0LL);
            AM_EXPECT(*filterData == 2.71828);

            // Test SizeOf works correctly for different pools
            AM_EXPECT(amMemory->SizeOf(eMemoryPoolKind_Engine, enginePtr) >= allocSize);
            AM_EXPECT(amMemory->SizeOf(eMemoryPoolKind_Amplimix, amplimixPtr) >= allocSize * 2);
            AM_EXPECT(amMemory->SizeOf(eMemoryPoolKind_SoundData, soundDataPtr) >= allocSize);
            AM_EXPECT(amMemory->SizeOf(eMemoryPoolKind_Filtering, filteringPtr) >= allocSize / 2);

            // Test that freeing from one pool doesn't affect others
            AmSize engineBeforeFree = amMemory->TotalReservedMemorySize(eMemoryPoolKind_Engine);
            AmSize amplimixBeforeFree = amMemory->TotalReservedMemorySize(eMemoryPoolKind_Amplimix);

            amMemory->Free(eMemoryPoolKind_Engine, enginePtr);

            // Verify Engine pool memory decreased but others remained the same
            AM_EXPECT(amMemory->TotalReservedMemorySize(eMemoryPoolKind_Engine) < engineBeforeFree);
            AM_EXPECT(amMemory->TotalReservedMemorySize(eMemoryPoolKind_Amplimix) == amplimixBeforeFree);
            AM_EXPECT(amMemory->TotalReservedMemorySize(eMemoryPoolKind_SoundData) > initialSoundData);
            AM_EXPECT(amMemory->TotalReservedMemorySize(eMemoryPoolKind_Filtering) > initialFiltering);

            // Verify data in other pools is still intact
            AM_EXPECT(*amplimixData == 3.14159f);
            AM_EXPECT(*soundData == 0x123456789ABCDEF0LL);
            AM_EXPECT(*filterData == 2.71828);

            // Test reallocation within the same pool
            AmVoidPtr newAmplimixPtr = amMemory->Realloc(eMemoryPoolKind_Amplimix, amplimixPtr, allocSize * 4, __FILE__, __LINE__);
            AM_EXPECT(newAmplimixPtr != nullptr);

            // Verify data is preserved during reallocation
            auto* newAmplimixData = static_cast<AmReal32*>(newAmplimixPtr);
            AM_EXPECT(*newAmplimixData == 3.14159f);

            // Test cross-pool reallocation is not allowed (should treat as separate malloc/free)
            // Note: This behavior depends on implementation, but generally realloc should only work within the same pool

            // Test total memory usage calculation
            AmSize totalBefore = amMemory->TotalReservedMemorySize();
            AmVoidPtr codecPtr = amMemory->Malloc(eMemoryPoolKind_Codec, 256, __FILE__, __LINE__);
            AM_EXPECT(codecPtr != nullptr);
            AmSize totalAfter = amMemory->TotalReservedMemorySize();
            AM_EXPECT(totalAfter > totalBefore);

            // Test that each pool tracks its own allocations correctly
            AmSize codecMemory = amMemory->TotalReservedMemorySize(eMemoryPoolKind_Codec);
            AM_EXPECT(codecMemory >= 256);

            // Test pool boundary conditions - ensure we can allocate from all available pools
            for (AmUInt8 poolIdx = 0; poolIdx < static_cast<AmUInt8>(eMemoryPoolKind_COUNT); ++poolIdx)
            {
                const auto poolKind = static_cast<eMemoryPoolKind>(poolIdx);
                AmSize poolSizeBefore = amMemory->TotalReservedMemorySize(poolKind);

                AmVoidPtr testPtr = amMemory->Malloc(poolKind, 128, __FILE__, __LINE__);
                AM_EXPECT(testPtr != nullptr);

                AmSize poolSizeAfter = amMemory->TotalReservedMemorySize(poolKind);
                AM_EXPECT(poolSizeAfter > poolSizeBefore);

                amMemory->Free(poolKind, testPtr);
            }

            // Clean up remaining allocations
            amMemory->Free(eMemoryPoolKind_Amplimix, newAmplimixPtr);
            amMemory->Free(eMemoryPoolKind_SoundData, soundDataPtr);
            amMemory->Free(eMemoryPoolKind_Filtering, filteringPtr);
            amMemory->Free(eMemoryPoolKind_Codec, codecPtr);

            // Verify all pools return to their initial state (or close to it)
            AM_EXPECT(amMemory->TotalReservedMemorySize(eMemoryPoolKind_Engine) == initialEngine);
            AM_EXPECT(amMemory->TotalReservedMemorySize(eMemoryPoolKind_Amplimix) == initialAmplimix);
            AM_EXPECT(amMemory->TotalReservedMemorySize(eMemoryPoolKind_SoundData) == initialSoundData);
            AM_EXPECT(amMemory->TotalReservedMemorySize(eMemoryPoolKind_Filtering) == initialFiltering);
            AM_EXPECT(amMemory->TotalReservedMemorySize(eMemoryPoolKind_Codec) == initialCodec);

            MemoryManager::Deinitialize();
        }
    };

    AM_REGISTER_TEST(core_memory, memory_pool_isolation);
} // namespace SparkyStudios::Audio::Amplitude::Tests
