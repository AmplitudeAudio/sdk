// Copyright (c) 2021-present Sparky Studios. All rights reserved.
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

#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>

#include <Utils/SmMalloc/smmalloc.h>

#if defined(AM_SIMD_INTRINSICS)
#include <xsimd/xsimd.hpp>
#endif // defined(AM_SIMD_INTRINSICS)

#include <mutex>
#include <sstream>
#include <unordered_map>

namespace SparkyStudios::Audio::Amplitude
{
    static MemoryManager* gMemManager = nullptr;

#if !defined(AM_NO_MEMORY_STATS)
    /**
     * Adds the given bytes to the pool usage, and raises the peak when a new high is reached.
     */
    static void RaiseUsage(MemoryPoolStats& stats, AmSize bytes)
    {
        const AmSize current = stats.currentMemoryUsed.fetch_add(bytes, std::memory_order_relaxed) + bytes;

        AmSize peak = stats.maxMemoryUsed.load(std::memory_order_relaxed);
        while (peak < current && !stats.maxMemoryUsed.compare_exchange_weak(peak, current, std::memory_order_relaxed))
        {}
    }
#endif

#if !defined(AM_NO_MEMORY_STATS)
    MemoryPoolStats::MemoryPoolStats(eMemoryPoolKind kind)
        : pool(kind)
    {
        currentMemoryUsed.store(0);
        maxMemoryUsed.store(0);
        allocCount.store(0);
        freeCount.store(0);
    }

    MemoryPoolStats::MemoryPoolStats(const MemoryPoolStats& copy)
        : MemoryPoolStats(copy.pool)
    {
        currentMemoryUsed.store(copy.currentMemoryUsed.load());
        maxMemoryUsed.store(copy.maxMemoryUsed.load());
        allocCount.store(copy.allocCount.load());
        freeCount.store(copy.freeCount.load());
    }

    MemoryPoolStats& MemoryPoolStats::operator=(const MemoryPoolStats& other)
    {
        pool = other.pool;
        currentMemoryUsed.store(other.currentMemoryUsed.load());
        maxMemoryUsed.store(other.maxMemoryUsed.load());
        allocCount.store(other.allocCount.load());
        freeCount.store(other.freeCount.load());

        return *this;
    }
#endif

    DefaultMemoryAllocator::DefaultMemoryAllocator(AmUInt32 bucketsCount, AmSize bucketSizeInBytes)
        : MemoryAllocator()
        , _allocators()
    {
        for (auto& _allocator : _allocators)
            _allocator = _sm_allocator_create(bucketsCount, bucketSizeInBytes);
    }

    DefaultMemoryAllocator::~DefaultMemoryAllocator()
    {
        for (auto& allocator : _allocators)
        {
            _sm_allocator_destroy(static_cast<sm_allocator>(allocator));
            allocator = nullptr;
        }
    }

    AmVoidPtr DefaultMemoryAllocator::Malloc(eMemoryPoolKind pool, AmSize size)
    {
#if defined(AM_SIMD_INTRINSICS)
        return _sm_malloc(static_cast<sm_allocator>(_allocators[static_cast<AmUInt32>(pool)]), size, xsimd::best_arch::alignment());
#else
        return _sm_malloc(static_cast<sm_allocator>(_allocators[static_cast<AmUInt32>(pool)]), size, 16);
#endif
    }

    AmVoidPtr DefaultMemoryAllocator::Realloc(eMemoryPoolKind pool, AmVoidPtr address, AmSize size)
    {
#if defined(AM_SIMD_INTRINSICS)
        return _sm_realloc(
            static_cast<sm_allocator>(_allocators[static_cast<AmUInt32>(pool)]), address, size, xsimd::best_arch::alignment());
#else
        return _sm_realloc(static_cast<sm_allocator>(_allocators[static_cast<AmUInt32>(pool)]), address, size, 16);
#endif
    }

    AmVoidPtr DefaultMemoryAllocator::Malign(eMemoryPoolKind pool, AmSize size, AmUInt32 alignment)
    {
        return _sm_malloc(static_cast<sm_allocator>(_allocators[static_cast<AmUInt32>(pool)]), size, alignment);
    }

    AmVoidPtr DefaultMemoryAllocator::Realign(eMemoryPoolKind pool, AmVoidPtr address, AmSize size, AmUInt32 alignment)
    {
        return _sm_realloc(static_cast<sm_allocator>(_allocators[static_cast<AmUInt32>(pool)]), address, size, alignment);
    }

    void DefaultMemoryAllocator::Free(eMemoryPoolKind pool, AmVoidPtr address)
    {
        _sm_free(static_cast<sm_allocator>(_allocators[static_cast<AmUInt32>(pool)]), address);
    }

    AmSize DefaultMemoryAllocator::SizeOf(eMemoryPoolKind pool, AmVoidPtr address)
    {
        return _sm_msize(static_cast<sm_allocator>(_allocators[static_cast<AmUInt32>(pool)]), address);
    }

    void MemoryManager::Initialize(std::unique_ptr<MemoryAllocator> allocator)
    {
        if (!IsInitialized())
            gMemManager = new MemoryManager(std::move(allocator));
    }

    void MemoryManager::Deinitialize()
    {
        if (gMemManager != nullptr)
        {
            delete gMemManager;
            gMemManager = nullptr;
        }
    }

    bool MemoryManager::IsInitialized()
    {
        return gMemManager != nullptr;
    }

    MemoryManager* MemoryManager::GetInstance()
    {
        AMPLITUDE_ASSERT(gMemManager != nullptr);
        return gMemManager;
    }

    MemoryManager::MemoryManager(std::unique_ptr<MemoryAllocator> allocator)
        : _allocator(std::move(allocator))
#if !defined(AM_NO_MEMORY_STATS)
        , _memPoolsStats()
#endif
#if defined(AM_MEMORY_TRACKING)
        , _memAllocations()
#endif
    {
        if (_allocator == nullptr)
            _allocator = std::make_unique<DefaultMemoryAllocator>(4, 16 * 1024 * 1024);

#if !defined(AM_NO_MEMORY_STATS)
        for (AmUInt32 i = 0; i < eMemoryPoolKind_COUNT; ++i)
            _memPoolsStats[i].pool = static_cast<eMemoryPoolKind>(i);
#endif
    }

    MemoryManager::~MemoryManager()
    {
#if defined(AM_MEMORY_TRACKING)
        // Clear the allocations set to prevent container leak
        _memAllocations.clear();
#endif

        _allocator.reset(nullptr);
    }

#if !defined(AM_NO_MEMORY_STATS)
    void MemoryManager::OnAllocate(
        eMemoryPoolKind pool, AmVoidPtr address, [[maybe_unused]] const char* file, [[maybe_unused]] AmUInt32 line)
    {
        if (address == nullptr)
            return;

        const AmSize size = _allocator->SizeOf(pool, address);

        MemoryPoolStats& stats = _memPoolsStats[pool];
        RaiseUsage(stats, size);
        stats.allocCount.fetch_add(1, std::memory_order_relaxed);

#if defined(AM_MEMORY_TRACKING)
        AddAllocation({ pool, address, size, file, line });
#endif
    }

    void MemoryManager::OnReallocate(
        eMemoryPoolKind pool,
        AmVoidPtr oldAddress,
        AmSize oldSize,
        AmVoidPtr address,
        AmSize requestedSize,
        [[maybe_unused]] const char* file,
        [[maybe_unused]] AmUInt32 line)
    {
        if (address == nullptr)
        {
            // A zero-sized request releases the block. Otherwise the request failed and the original block is still valid.
            if (oldAddress != nullptr && requestedSize == 0)
                OnFree(pool, oldAddress, oldSize);

            return;
        }

        if (oldAddress == nullptr)
        {
            OnAllocate(pool, address, file, line);
            return;
        }

        const AmSize newSize = _allocator->SizeOf(pool, address);

        MemoryPoolStats& stats = _memPoolsStats[pool];
        if (newSize >= oldSize)
            RaiseUsage(stats, newSize - oldSize);
        else
            stats.currentMemoryUsed.fetch_sub(oldSize - newSize, std::memory_order_relaxed);

#if defined(AM_MEMORY_TRACKING)
        {
            std::lock_guard<std::mutex> lock(_allocationsMutex);
            if (const auto& it = _memAllocations.find({ pool, oldAddress }); it != _memAllocations.end())
                _memAllocations.erase(it);
            _memAllocations.insert({ pool, address, newSize, file, line });
        }
#endif
    }

    void MemoryManager::OnFree(eMemoryPoolKind pool, [[maybe_unused]] AmVoidPtr address, AmSize size)
    {
        MemoryPoolStats& stats = _memPoolsStats[pool];
        stats.currentMemoryUsed.fetch_sub(size, std::memory_order_relaxed);
        stats.freeCount.fetch_add(1, std::memory_order_relaxed);

#if defined(AM_MEMORY_TRACKING)
        RemoveAllocation({ pool, address });
#endif
    }
#endif

#if defined(AM_MEMORY_TRACKING)
    void MemoryManager::RemoveAllocation(const Allocation& allocation)
    {
        std::lock_guard<std::mutex> lock(_allocationsMutex);
        if (const auto& it = _memAllocations.find(allocation); it != _memAllocations.end())
            _memAllocations.erase(it);
    }

    void MemoryManager::AddAllocation(const Allocation& allocation)
    {
        std::lock_guard<std::mutex> lock(_allocationsMutex);
        _memAllocations.insert(allocation);
    }
#endif

    AmVoidPtr MemoryManager::Malloc(eMemoryPoolKind pool, AmSize size, const char* file, AmUInt32 line)
    {
        AmVoidPtr ptr = _allocator->Malloc(pool, size);

#if !defined(AM_NO_MEMORY_STATS)
        OnAllocate(pool, ptr, file, line);
#endif

        return ptr;
    }

    AmVoidPtr MemoryManager::Malign(eMemoryPoolKind pool, AmSize size, AmUInt32 alignment, const char* file, AmUInt32 line)
    {
        AmVoidPtr ptr = _allocator->Malign(pool, size, alignment);

#if !defined(AM_NO_MEMORY_STATS)
        OnAllocate(pool, ptr, file, line);
#endif

        return ptr;
    }

    AmVoidPtr MemoryManager::Realloc(eMemoryPoolKind pool, AmVoidPtr address, AmSize size, const char* file, AmUInt32 line)
    {
#if !defined(AM_NO_MEMORY_STATS)
        const AmSize oldSize = _allocator->SizeOf(pool, address);
#endif

        AmVoidPtr ptr = _allocator->Realloc(pool, address, size);

#if !defined(AM_NO_MEMORY_STATS)
        OnReallocate(pool, address, oldSize, ptr, size, file, line);
#endif

        return ptr;
    }

    AmVoidPtr MemoryManager::Realign(
        eMemoryPoolKind pool, AmVoidPtr address, AmSize size, AmUInt32 alignment, const char* file, AmUInt32 line)
    {
#if !defined(AM_NO_MEMORY_STATS)
        const AmSize oldSize = _allocator->SizeOf(pool, address);
#endif

        AmVoidPtr ptr = _allocator->Realign(pool, address, size, alignment);

#if !defined(AM_NO_MEMORY_STATS)
        OnReallocate(pool, address, oldSize, ptr, size, file, line);
#endif

        return ptr;
    }

    void MemoryManager::Free(eMemoryPoolKind pool, AmVoidPtr address)
    {
#if !defined(AM_NO_MEMORY_STATS)
        // The usable size must be read before the block is released.
        const AmSize size = _allocator->SizeOf(pool, address);
#endif

        _allocator->Free(pool, address);

#if !defined(AM_NO_MEMORY_STATS)
        if (address != nullptr)
            OnFree(pool, address, size);
#endif
    }

    AmSize MemoryManager::SizeOf(eMemoryPoolKind pool, AmVoidPtr address) const
    {
        return _allocator->SizeOf(pool, address);
    }

#if !defined(AM_NO_MEMORY_STATS)
    AmSize MemoryManager::TotalReservedMemorySize(eMemoryPoolKind pool) const
    {
        return _memPoolsStats[pool].currentMemoryUsed.load(std::memory_order_relaxed);
    }

    AmSize MemoryManager::TotalReservedMemorySize() const
    {
        AmSize total = 0;
        for (const auto& stats : _memPoolsStats)
            total += stats.currentMemoryUsed.load(std::memory_order_relaxed);

        return total;
    }

    AmString MemoryManager::GetMemoryPoolName(const eMemoryPoolKind pool)
    {
        static std::unordered_map<eMemoryPoolKind, std::string> gMemoryPoolNames = {
            { eMemoryPoolKind_Amplimix, "Amplimix" },   { eMemoryPoolKind_Codec, "Codec" },         { eMemoryPoolKind_Engine, "Engine" },
            { eMemoryPoolKind_Filtering, "Filtering" }, { eMemoryPoolKind_SoundData, "SoundData" }, { eMemoryPoolKind_IO, "IO" },
            { eMemoryPoolKind_Default, "Default" },
        };

        if (const auto it = gMemoryPoolNames.find(pool); it != gMemoryPoolNames.end())
            return it->second;

        return "Unknown";
    }

    const MemoryPoolStats& MemoryManager::GetStats(eMemoryPoolKind pool) const
    {
        return _memPoolsStats.at(pool);
    }

    AmString MemoryManager::InspectMemoryLeaks() const
    {
        std::stringstream ss;

#if defined(AM_MEMORY_TRACKING)
        std::lock_guard<std::mutex> lock(_allocationsMutex);

        if (_memAllocations.empty())
            return "No memory leaks detected";

        ss << "=== Memory leaks detected ===\n\n";

        for (auto&& allocation : _memAllocations)
        {
            ss << "Pool: " << GetMemoryPoolName(allocation.pool) << std::endl;
            ss << "  Address: " << allocation.address << std::endl;
            ss << "  Size: " << allocation.size << std::endl;
            ss << "  File: " << allocation.file << std::endl;
            ss << "  Line: " << allocation.line << std::endl << std::endl;
        }
#else
        // Without allocation tracking, only the outstanding byte counts per pool are known.
        if (TotalReservedMemorySize() == 0)
            return "No memory leaks detected";

        ss << "=== Memory leaks detected ===\n\n";

        for (const auto& stats : _memPoolsStats)
        {
            const AmSize used = stats.currentMemoryUsed.load(std::memory_order_relaxed);
            if (used == 0)
                continue;

            ss << "Pool: " << GetMemoryPoolName(stats.pool) << std::endl;
            ss << "  Size: " << used << " (allocation tracking is disabled, no per-allocation details)" << std::endl << std::endl;
        }
#endif

        return ss.str();
    }
#endif

    ScopedMemoryAllocation::ScopedMemoryAllocation(eMemoryPoolKind pool, AmSize size, const char* file, AmUInt32 line)
    {
        _pool = pool;
        _address = amMemory->Malloc(_pool, size, file, line);
    }

    ScopedMemoryAllocation::ScopedMemoryAllocation(eMemoryPoolKind pool, AmSize size, AmUInt32 alignment, const char* file, AmUInt32 line)
    {
        _pool = pool;
        _address = amMemory->Malign(_pool, size, alignment, file, line);
    }

    ScopedMemoryAllocation::ScopedMemoryAllocation(ScopedMemoryAllocation&& other) noexcept
    {
        _pool = other._pool;
        _address = other._address;
        other._address = nullptr;
    }

    ScopedMemoryAllocation& ScopedMemoryAllocation::operator=(ScopedMemoryAllocation&& other) noexcept
    {
        if (this != &other)
        {
            // Release current resource
            if (_address != nullptr)
            {
                ampoolfree(_pool, _address);
            }

            // Transfer ownership
            _pool = other._pool;
            _address = other._address;
            other._address = nullptr;
        }

        return *this;
    }

    ScopedMemoryAllocation::~ScopedMemoryAllocation()
    {
        if (_address == nullptr)
            return;

        ampoolfree(_pool, _address);
        _address = nullptr;
    }
} // namespace SparkyStudios::Audio::Amplitude
