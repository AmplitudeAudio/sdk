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

#include <memory>
#include <mutex>
#include <unordered_map>

#if defined(AM_SIMD_INTRINSICS)
#include <xsimd/xsimd.hpp>
#endif // defined(AM_SIMD_INTRINSICS)

using namespace SparkyStudios::Audio::Amplitude;

namespace
{
    // Simple tracking allocator using std::allocator for testing
    class TestMemoryAllocator : public MemoryAllocator
    {
    public:
        struct AllocationInfo
        {
            AmSize size;
            AmUInt32 alignment;
        };

        TestMemoryAllocator()
            : _allocCounts{}
            , _freeCounts{}
            , _totalAllocated{}
            , _mutex()
        {}

        ~TestMemoryAllocator() override = default;

        AmVoidPtr Malloc(eMemoryPoolKind pool, AmSize size) override
        {
            std::lock_guard<std::mutex> lock(_mutex);

            _allocCounts[static_cast<AmUInt32>(pool)]++;
            _totalAllocated[static_cast<AmUInt32>(pool)] += size;

#if defined(AM_SIMD_INTRINSICS)
            AmUInt32 alignment = xsimd::best_arch::alignment();
#else
            AmUInt32 alignment = 16;
#endif

            return AllocateAligned(size, alignment, pool);
        }

        AmVoidPtr Realloc(eMemoryPoolKind pool, AmVoidPtr address, AmSize size) override
        {
            std::lock_guard<std::mutex> lock(_mutex);

            if (address == nullptr)
            {
                _allocCounts[static_cast<AmUInt32>(pool)]++;
                _totalAllocated[static_cast<AmUInt32>(pool)] += size;

#if defined(AM_SIMD_INTRINSICS)
                AmUInt32 alignment = xsimd::best_arch::alignment();
#else
                AmUInt32 alignment = 16;
#endif
                return AllocateAligned(size, alignment, pool);
            }

            // Find the original allocation info
            auto it = _allocations.find(address);
            if (it == _allocations.end())
            {
                return nullptr; // Invalid address
            }

            AmSize oldSize = it->second.size;
            AmUInt32 alignment = it->second.alignment;

            // Allocate new memory
            AmVoidPtr newPtr = AllocateAligned(size, alignment, pool);
            if (newPtr != nullptr)
            {
                // Copy old data
                std::memcpy(newPtr, address, std::min(oldSize, size));

                // Free old memory
                FreeAligned(address);
                _allocations.erase(it);
            }

            return newPtr;
        }

        AmVoidPtr Malign(eMemoryPoolKind pool, AmSize size, AmUInt32 alignment) override
        {
            std::lock_guard<std::mutex> lock(_mutex);

            _allocCounts[static_cast<AmUInt32>(pool)]++;
            _totalAllocated[static_cast<AmUInt32>(pool)] += size;

            return AllocateAligned(size, alignment, pool);
        }

        AmVoidPtr Realign(eMemoryPoolKind pool, AmVoidPtr address, AmSize size, AmUInt32 alignment) override
        {
            std::lock_guard<std::mutex> lock(_mutex);

            if (address == nullptr)
            {
                _allocCounts[static_cast<AmUInt32>(pool)]++;
                _totalAllocated[static_cast<AmUInt32>(pool)] += size;
                return AllocateAligned(size, alignment, pool);
            }

            // Find the original allocation info
            auto it = _allocations.find(address);
            if (it == _allocations.end())
            {
                return nullptr; // Invalid address
            }

            AmSize oldSize = it->second.size;

            // Allocate new aligned memory
            AmVoidPtr newPtr = AllocateAligned(size, alignment, pool);
            if (newPtr != nullptr)
            {
                // Copy old data
                std::memcpy(newPtr, address, std::min(oldSize, size));

                // Free old memory
                FreeAligned(address);
                _allocations.erase(it);
            }

            return newPtr;
        }

        void Free(eMemoryPoolKind pool, AmVoidPtr address) override
        {
            if (address == nullptr)
                return;

            std::lock_guard<std::mutex> lock(_mutex);

            _freeCounts[static_cast<AmUInt32>(pool)]++;

            auto it = _allocations.find(address);
            if (it != _allocations.end())
            {
                FreeAligned(address);
                _allocations.erase(it);
            }
        }

        AmSize SizeOf(eMemoryPoolKind pool, AmVoidPtr address) override
        {
            std::lock_guard<std::mutex> lock(_mutex);

            auto it = _allocations.find(address);
            if (it != _allocations.end())
            {
                return it->second.size;
            }
            return 0;
        }

        // Test interface methods
        AmUInt32 GetAllocCount(eMemoryPoolKind pool) const
        {
            std::lock_guard<std::mutex> lock(_mutex);
            return _allocCounts[static_cast<AmUInt32>(pool)];
        }

        AmUInt32 GetFreeCount(eMemoryPoolKind pool) const
        {
            std::lock_guard<std::mutex> lock(_mutex);
            return _freeCounts[static_cast<AmUInt32>(pool)];
        }

        AmSize GetTotalAllocated(eMemoryPoolKind pool) const
        {
            std::lock_guard<std::mutex> lock(_mutex);
            return _totalAllocated[static_cast<AmUInt32>(pool)];
        }

        AmSize GetActiveAllocations() const
        {
            std::lock_guard<std::mutex> lock(_mutex);
            return _allocations.size();
        }

    private:
        AmVoidPtr AllocateAligned(AmSize size, AmUInt32 alignment, eMemoryPoolKind pool)
        {
            // Use aligned_alloc if available, otherwise fallback to manual alignment
#if defined(_MSC_VER)
            AmVoidPtr ptr = _aligned_malloc(size, alignment);
#else
            AmVoidPtr ptr = std::aligned_alloc(alignment, AM_VALUE_ALIGN(size, alignment));
#endif

            if (ptr != nullptr)
            {
                _allocations[ptr] = { size, alignment };
            }

            return ptr;
        }

        void FreeAligned(AmVoidPtr ptr)
        {
            if (ptr == nullptr)
                return;

#if defined(_MSC_VER)
            _aligned_free(ptr);
#else
            std::free(ptr);
#endif
        }

        mutable std::mutex _mutex;
        std::unordered_map<AmVoidPtr, AllocationInfo> _allocations;
        AmUInt32 _allocCounts[eMemoryPoolKind_COUNT];
        AmUInt32 _freeCounts[eMemoryPoolKind_COUNT];
        AmSize _totalAllocated[eMemoryPoolKind_COUNT];
    };
} // namespace

void MemoryTestCase::Run()
{
    // Test custom allocator creation and usage
    auto customAllocator = std::make_unique<TestMemoryAllocator>();
    TestMemoryAllocator* allocatorPtr = customAllocator.get();

    // Verify no allocations initially
    AM_EXPECT(allocatorPtr->GetActiveAllocations() == 0);

    // Initialize memory manager with custom allocator
    MemoryManager::Initialize(std::move(customAllocator));
    AM_EXPECT(MemoryManager::IsInitialized());

    // Test that custom allocator is being used
    constexpr AmSize testSize = 1024;
    AmUInt32 initialAllocCount = allocatorPtr->GetAllocCount(eMemoryPoolKind_Default);
    AmSize initialTotalAllocated = allocatorPtr->GetTotalAllocated(eMemoryPoolKind_Default);

    AmVoidPtr ptr = amMemory->Malloc(eMemoryPoolKind_Default, testSize, __FILE__, __LINE__);
    AM_EXPECT(ptr != nullptr);

    // Verify custom allocator was called
    AM_EXPECT(allocatorPtr->GetAllocCount(eMemoryPoolKind_Default) == initialAllocCount + 1);
    AM_EXPECT(allocatorPtr->GetTotalAllocated(eMemoryPoolKind_Default) >= initialTotalAllocated + testSize);
    AM_EXPECT(allocatorPtr->GetActiveAllocations() > 0);

    // Test writing to allocated memory
    auto* intPtr = static_cast<AmInt32*>(ptr);
    *intPtr = 0x12345678;
    AM_EXPECT(*intPtr == 0x12345678);

    // Test SizeOf with custom allocator
    AM_EXPECT(amMemory->SizeOf(eMemoryPoolKind_Default, ptr) == testSize);

    // Test custom allocator free tracking
    AmUInt32 initialFreeCount = allocatorPtr->GetFreeCount(eMemoryPoolKind_Default);
    AmSize activeAllocsBefore = allocatorPtr->GetActiveAllocations();
    amMemory->Free(eMemoryPoolKind_Default, ptr);
    AM_EXPECT(allocatorPtr->GetFreeCount(eMemoryPoolKind_Default) == initialFreeCount + 1);
    AM_EXPECT(allocatorPtr->GetActiveAllocations() == activeAllocsBefore - 1);

    // Test aligned allocation through custom allocator
    AmUInt32 engineInitialAllocs = allocatorPtr->GetAllocCount(eMemoryPoolKind_Engine);
    AmVoidPtr alignedPtr = amMemory->Malign(eMemoryPoolKind_Engine, 512, 64, __FILE__, __LINE__);
    AM_EXPECT(alignedPtr != nullptr);
    AM_EXPECT(reinterpret_cast<uintptr_t>(alignedPtr) % 64 == 0);
    AM_EXPECT(allocatorPtr->GetAllocCount(eMemoryPoolKind_Engine) == engineInitialAllocs + 1);
    AM_EXPECT(amMemory->SizeOf(eMemoryPoolKind_Engine, alignedPtr) == 512);

    amMemory->Free(eMemoryPoolKind_Engine, alignedPtr);

    // Test reallocation through custom allocator
    AmVoidPtr reallocPtr = amMemory->Malloc(eMemoryPoolKind_SoundData, 256, __FILE__, __LINE__);
    AM_EXPECT(reallocPtr != nullptr);

    auto* testData = static_cast<AmReal32*>(reallocPtr);
    testData[0] = 1.5f;
    testData[1] = 2.5f;

    AmVoidPtr newReallocPtr = amMemory->Realloc(eMemoryPoolKind_SoundData, reallocPtr, 512, __FILE__, __LINE__);
    AM_EXPECT(newReallocPtr != nullptr);
    AM_EXPECT(amMemory->SizeOf(eMemoryPoolKind_SoundData, newReallocPtr) == 512);

    // Verify data preservation
    auto* newTestData = static_cast<AmReal32*>(newReallocPtr);
    AM_EXPECT(newTestData[0] == 1.5f);
    AM_EXPECT(newTestData[1] == 2.5f);

    amMemory->Free(eMemoryPoolKind_SoundData, newReallocPtr);

    // Test that different pools track independently in custom allocator
    AmUInt32 filteringAllocs = allocatorPtr->GetAllocCount(eMemoryPoolKind_Filtering);
    AmUInt32 codecAllocs = allocatorPtr->GetAllocCount(eMemoryPoolKind_Codec);
    AmUInt32 ioAllocs = allocatorPtr->GetAllocCount(eMemoryPoolKind_IO);

    AmVoidPtr filterPtr = amMemory->Malloc(eMemoryPoolKind_Filtering, 128, __FILE__, __LINE__);
    AmVoidPtr codecPtr = amMemory->Malloc(eMemoryPoolKind_Codec, 256, __FILE__, __LINE__);
    AmVoidPtr ioPtr = amMemory->Malloc(eMemoryPoolKind_IO, 512, __FILE__, __LINE__);

    AM_EXPECT(filterPtr != nullptr);
    AM_EXPECT(codecPtr != nullptr);
    AM_EXPECT(ioPtr != nullptr);

    AM_EXPECT(allocatorPtr->GetAllocCount(eMemoryPoolKind_Filtering) == filteringAllocs + 1);
    AM_EXPECT(allocatorPtr->GetAllocCount(eMemoryPoolKind_Codec) == codecAllocs + 1);
    AM_EXPECT(allocatorPtr->GetAllocCount(eMemoryPoolKind_IO) == ioAllocs + 1);

    // Verify sizes are tracked correctly
    AM_EXPECT(amMemory->SizeOf(eMemoryPoolKind_Filtering, filterPtr) == 128);
    AM_EXPECT(amMemory->SizeOf(eMemoryPoolKind_Codec, codecPtr) == 256);
    AM_EXPECT(amMemory->SizeOf(eMemoryPoolKind_IO, ioPtr) == 512);

    // Clean up
    amMemory->Free(eMemoryPoolKind_Filtering, filterPtr);
    amMemory->Free(eMemoryPoolKind_Codec, codecPtr);
    amMemory->Free(eMemoryPoolKind_IO, ioPtr);

    // Test various alignment values
    for (AmUInt32 alignment = 16; alignment <= 256; alignment *= 2)
    {
        AmVoidPtr alignTestPtr = amMemory->Malign(eMemoryPoolKind_Amplimix, 1024, alignment, __FILE__, __LINE__);
        AM_EXPECT(alignTestPtr != nullptr);
        AM_EXPECT(reinterpret_cast<uintptr_t>(alignTestPtr) % alignment == 0);
        AM_EXPECT(amMemory->SizeOf(eMemoryPoolKind_Amplimix, alignTestPtr) == 1024);
        amMemory->Free(eMemoryPoolKind_Amplimix, alignTestPtr);
    }

    // Test allocation count consistency
    for (AmUInt8 poolIdx = 0; poolIdx < static_cast<AmUInt8>(eMemoryPoolKind_COUNT); ++poolIdx)
    {
        const auto poolKind = static_cast<eMemoryPoolKind>(poolIdx);
        AmUInt32 allocCount = allocatorPtr->GetAllocCount(poolKind);
        AmUInt32 freeCount = allocatorPtr->GetFreeCount(poolKind);

        // Alloc count should be >= free count (some allocations might still be active)
        AM_EXPECT(allocCount >= freeCount);
    }

    // Test large allocation through custom allocator
    constexpr AmSize largeSize = 1024 * 1024; // 1MB
    AmVoidPtr largePtr = amMemory->Malloc(eMemoryPoolKind_Default, largeSize, __FILE__, __LINE__);
    AM_EXPECT(largePtr != nullptr);
    AM_EXPECT(amMemory->SizeOf(eMemoryPoolKind_Default, largePtr) == largeSize);

    // Test writing to large allocation
    auto* largeData = static_cast<AmUInt32*>(largePtr);
    const AmSize numInts = largeSize / sizeof(AmUInt32);
    for (AmSize i = 0; i < numInts; i += 1000) // Test every 1000th element to avoid long test times
    {
        largeData[i] = static_cast<AmUInt32>(i);
    }

    for (AmSize i = 0; i < numInts; i += 1000)
    {
        AM_EXPECT(largeData[i] == static_cast<AmUInt32>(i));
    }

    amMemory->Free(eMemoryPoolKind_Default, largePtr);

    // Test that custom allocator handles null pointer operations gracefully
    AmVoidPtr nullRealloc = amMemory->Realloc(eMemoryPoolKind_Default, nullptr, 128, __FILE__, __LINE__);
    AM_EXPECT(nullRealloc != nullptr);
    AM_EXPECT(amMemory->SizeOf(eMemoryPoolKind_Default, nullRealloc) == 128);
    amMemory->Free(eMemoryPoolKind_Default, nullRealloc);

    AmVoidPtr nullRealign = amMemory->Realign(eMemoryPoolKind_Default, nullptr, 128, 32, __FILE__, __LINE__);
    AM_EXPECT(nullRealign != nullptr);
    AM_EXPECT(reinterpret_cast<uintptr_t>(nullRealign) % 32 == 0);
    AM_EXPECT(amMemory->SizeOf(eMemoryPoolKind_Default, nullRealign) == 128);
    amMemory->Free(eMemoryPoolKind_Default, nullRealign);

    // Test realign functionality
    AmVoidPtr realignPtr = amMemory->Malign(eMemoryPoolKind_Default, 64, 16, __FILE__, __LINE__);
    AM_EXPECT(realignPtr != nullptr);

    auto* realignData = static_cast<AmReal64*>(realignPtr);
    *realignData = 3.14159;

    AmVoidPtr newRealignPtr = amMemory->Realign(eMemoryPoolKind_Default, realignPtr, 128, 64, __FILE__, __LINE__);
    AM_EXPECT(newRealignPtr != nullptr);
    AM_EXPECT(reinterpret_cast<uintptr_t>(newRealignPtr) % 64 == 0);
    AM_EXPECT(amMemory->SizeOf(eMemoryPoolKind_Default, newRealignPtr) == 128);

    auto* newRealignData = static_cast<AmReal64*>(newRealignPtr);
    AM_EXPECT(*newRealignData == 3.14159);

    amMemory->Free(eMemoryPoolKind_Default, newRealignPtr);

    // Test zero-size allocation
    AmVoidPtr zeroPtr = amMemory->Malloc(eMemoryPoolKind_Default, 0, __FILE__, __LINE__);
    if (zeroPtr != nullptr)
    {
        AM_EXPECT(amMemory->SizeOf(eMemoryPoolKind_Default, zeroPtr) == 0);
        amMemory->Free(eMemoryPoolKind_Default, zeroPtr);
    }

    MemoryManager::Deinitialize();
    AM_EXPECT(!MemoryManager::IsInitialized());
}
