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
    AM_TEST_CASE(MemoryTestCase, core_memory, scoped_memory_allocation)
    {
    public:
        void Run() override
        {
            // Initialize memory manager
            MemoryManager::Initialize(nullptr);

            AmSize initialMemorySize = amMemory->TotalReservedMemorySize(eMemoryPoolKind_Default);

            // Test basic scoped allocation
            {
                ScopedMemoryAllocation scoped(eMemoryPoolKind_Default, 1024, __FILE__, __LINE__);
                AM_EXPECT(scoped.Address() != nullptr);
                AM_EXPECT(amMemory->TotalReservedMemorySize(eMemoryPoolKind_Default) > initialMemorySize);

                // Test that we can write to the memory
                AmInt32* intPtr = scoped.PointerOf<AmInt32>();
                *intPtr = 42;
                AM_EXPECT(*intPtr == 42);

                // Test As() template method
                AmReal32* floatPtr = scoped.As<AmReal32*>();
                *floatPtr = 3.14159f;
                AM_EXPECT(*floatPtr == 3.14159f);
            } // Scoped allocation should be automatically freed here

            // Verify memory was freed
            AmSize finalMemorySize = amMemory->TotalReservedMemorySize(eMemoryPoolKind_Default);
            AM_EXPECT(finalMemorySize == initialMemorySize);

            // Test scoped aligned allocation
            AmSize initialAlignedMemory = amMemory->TotalReservedMemorySize(eMemoryPoolKind_Amplimix);
            {
                ScopedMemoryAllocation scopedAligned(eMemoryPoolKind_Amplimix, 512, 64, __FILE__, __LINE__);
                AM_EXPECT(scopedAligned.Address() != nullptr);
                AM_EXPECT(reinterpret_cast<uintptr_t>(scopedAligned.Address()) % 64 == 0);
                AM_EXPECT(amMemory->TotalReservedMemorySize(eMemoryPoolKind_Amplimix) > initialAlignedMemory);

                // Test writing to aligned memory
                AmReal32* alignedFloats = scopedAligned.PointerOf<AmReal32>();
                for (int i = 0; i < 128; ++i)
                    alignedFloats[i] = static_cast<AmReal32>(i) * 0.1f;

                // Verify values
                for (int i = 0; i < 128; ++i)
                    AM_EXPECT(alignedFloats[i] == static_cast<AmReal32>(i) * 0.1f);
            }

            // Verify aligned memory was freed
            AmSize finalAlignedMemory = amMemory->TotalReservedMemorySize(eMemoryPoolKind_Amplimix);
            AM_EXPECT(finalAlignedMemory == initialAlignedMemory);

            // Test move constructor
            AmSize enginePoolInitial = amMemory->TotalReservedMemorySize(eMemoryPoolKind_Engine);
            AmVoidPtr originalAddress = nullptr;
            {
                ScopedMemoryAllocation scoped1(eMemoryPoolKind_Engine, 256, __FILE__, __LINE__);
                originalAddress = scoped1.Address();
                AM_EXPECT(originalAddress != nullptr);

                // Move construct
                ScopedMemoryAllocation scoped2(std::move(scoped1));
                AM_EXPECT(scoped1.Address() == nullptr); // Original should be null after move
                AM_EXPECT(scoped2.Address() == originalAddress); // New should have the address

                // Test that memory is still accessible through moved object
                AmInt32* intPtr = scoped2.PointerOf<AmInt32>();
                *intPtr = 99;
                AM_EXPECT(*intPtr == 99);
            } // Only scoped2 should free the memory

            AmSize enginePoolFinal = amMemory->TotalReservedMemorySize(eMemoryPoolKind_Engine);
            AM_EXPECT(enginePoolFinal == enginePoolInitial);

            // Test move assignment operator
            AmSize ioPoolInitial = amMemory->TotalReservedMemorySize(eMemoryPoolKind_IO);
            {
                ScopedMemoryAllocation scoped1(eMemoryPoolKind_IO, 128, __FILE__, __LINE__);
                ScopedMemoryAllocation scoped2(eMemoryPoolKind_IO, 64, __FILE__, __LINE__);

                AmVoidPtr addr1 = scoped1.Address();
                AmVoidPtr addr2 = scoped2.Address();

                AM_EXPECT(addr1 != nullptr);
                AM_EXPECT(addr2 != nullptr);
                AM_EXPECT(addr1 != addr2);

                // Move assign scoped1 to scoped2
                scoped2 = std::move(scoped1);

                AM_EXPECT(scoped1.Address() == nullptr); // Original should be null
                AM_EXPECT(scoped2.Address() == addr1); // Should have the moved address

                // The old scoped2 memory should have been freed during move assignment
            }

            AmSize ioPoolFinal = amMemory->TotalReservedMemorySize(eMemoryPoolKind_IO);
            AM_EXPECT(ioPoolFinal == ioPoolInitial);

            // Test exception safety (even though this test framework doesn't use exceptions)
            // This ensures memory is properly cleaned up in all scenarios
            {
                ScopedMemoryAllocation scoped(eMemoryPoolKind_SoundData, 2048, __FILE__, __LINE__);
                AM_EXPECT(scoped.Address() != nullptr);

                // Simulate some work that might throw
                AmReal64* doublePtr = scoped.PointerOf<AmReal64>();
                for (int i = 0; i < 256; ++i)
                    doublePtr[i] = static_cast<AmReal64>(i) * 3.14159;

                // Verify values to ensure memory is working correctly
                for (int i = 0; i < 256; ++i)
                    AM_EXPECT(doublePtr[i] == static_cast<AmReal64>(i) * 3.14159);
            } // Memory should be automatically freed even if exceptions occurred

            MemoryManager::Deinitialize();
        }
    };

    AM_REGISTER_TEST(core_memory, scoped_memory_allocation);
} // namespace SparkyStudios::Audio::Amplitude::Tests
