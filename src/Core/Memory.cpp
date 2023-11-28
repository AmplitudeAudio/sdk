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

namespace SparkyStudios::Audio::Amplitude
{
    AM_API_PUBLIC MemoryManager* gMemManager = nullptr;

    MemoryManagerConfig::MemoryManagerConfig()
        : malloc(nullptr)
        , realloc(nullptr)
        , alignedMalloc(nullptr)
        , alignedRealloc(nullptr)
        , free(nullptr)
        , totalReservedMemorySize(nullptr)
        , sizeOf(nullptr)
        , bucketsCount(4)
        , bucketsSizeInBytes(16 * 1024 * 1024)
    {}

#if !defined(AM_NO_MEMORY_STATS)
    MemoryPoolStats::MemoryPoolStats(MemoryPoolKind kind)
        : pool(kind)
    {
        maxMemoryUsed.store(0);
        allocCount.store(0);
        freeCount.store(0);
    }

    MemoryPoolStats::MemoryPoolStats(const MemoryPoolStats& copy)
        : MemoryPoolStats(copy.pool)
    {
        maxMemoryUsed.store(copy.maxMemoryUsed.load());
        allocCount.store(copy.allocCount.load());
        freeCount.store(copy.freeCount.load());
    }

    MemoryPoolStats& MemoryPoolStats::operator=(const MemoryPoolStats& other)
    {
        pool = other.pool;
        maxMemoryUsed.store(other.maxMemoryUsed.load());
        allocCount.store(other.allocCount.load());
        freeCount.store(other.freeCount.load());

        return *this;
    }
#endif

    void MemoryManager::Initialize(const MemoryManagerConfig& config)
    {
        if (gMemManager == nullptr)
        {
            gMemManager = new MemoryManager(config);
        }

#if !defined(AM_NO_MEMORY_STATS)
        gMemManager->_memPoolsStats[MemoryPoolKind::Amplimix] = MemoryPoolStats(MemoryPoolKind::Amplimix);
        gMemManager->_memPoolsStats[MemoryPoolKind::Codec] = MemoryPoolStats(MemoryPoolKind::Codec);
        gMemManager->_memPoolsStats[MemoryPoolKind::Default] = MemoryPoolStats(MemoryPoolKind::Default);
        gMemManager->_memPoolsStats[MemoryPoolKind::Engine] = MemoryPoolStats(MemoryPoolKind::Engine);
        gMemManager->_memPoolsStats[MemoryPoolKind::Filtering] = MemoryPoolStats(MemoryPoolKind::Filtering);
        gMemManager->_memPoolsStats[MemoryPoolKind::IO] = MemoryPoolStats(MemoryPoolKind::IO);
        gMemManager->_memPoolsStats[MemoryPoolKind::SoundData] = MemoryPoolStats(MemoryPoolKind::SoundData);
#endif
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

    MemoryManager::MemoryManager(MemoryManagerConfig config)
        : _config(config)
    {
        if (config.malloc == nullptr)
        {
            _memAllocator = _sm_allocator_create(config.bucketsCount, config.bucketsSizeInBytes);
        }
        else
        {
            AMPLITUDE_ASSERT(
                config.realloc != nullptr && config.free != nullptr && config.alignedMalloc != nullptr &&
                config.alignedRealloc != nullptr && config.sizeOf != nullptr);

            _memAllocator = nullptr;
        }
    }

    MemoryManager::~MemoryManager()
    {
        if (_memAllocator != nullptr)
        {
            _sm_allocator_destroy(static_cast<sm_allocator>(_memAllocator));
        }
    }

    AmVoidPtr MemoryManager::Malloc(MemoryPoolKind pool, AmSize size, const char* file, AmUInt32 line)
    {
#if !defined(AM_NO_MEMORY_STATS)
        _memPoolsStats[pool].maxMemoryUsed.fetch_add(size, std::memory_order_relaxed);
        _memPoolsStats[pool].allocCount.fetch_add(1, std::memory_order_relaxed);
#endif

        if (_config.malloc != nullptr)
        {
            return _config.malloc(pool, size);
        }

        AmVoidPtr p = _sm_malloc(static_cast<sm_allocator>(_memAllocator), size, 0);
        _memPoolsData[pool].insert(p);
        return p;
    }

    AmVoidPtr MemoryManager::Malign(MemoryPoolKind pool, AmSize size, AmUInt32 alignment, const char* file, AmUInt32 line)
    {
#if !defined(AM_NO_MEMORY_STATS)
        _memPoolsStats[pool].maxMemoryUsed.fetch_add(size, std::memory_order_relaxed);
        _memPoolsStats[pool].allocCount.fetch_add(1, std::memory_order_relaxed);
#endif

        if (_config.alignedMalloc != nullptr)
        {
            return _config.alignedMalloc(pool, size, alignment);
        }

        AmVoidPtr p = _sm_malloc(static_cast<sm_allocator>(_memAllocator), size, alignment);
        _memPoolsData[pool].insert(p);
        return p;
    }

    AmVoidPtr MemoryManager::Realloc(MemoryPoolKind pool, AmVoidPtr address, AmSize size, const char* file, AmUInt32 line)
    {
        if (address == nullptr)
        {
#if !defined(AM_NO_MEMORY_STATS)
            _memPoolsStats[pool].maxMemoryUsed.fetch_add(size, std::memory_order_relaxed);
            _memPoolsStats[pool].allocCount.fetch_add(1, std::memory_order_relaxed);
#endif
        }

        if (_config.realloc != nullptr)
        {
            return _config.realloc(pool, address, size);
        }

        AmVoidPtr p = _sm_realloc(static_cast<sm_allocator>(_memAllocator), address, size, 0);
        if (const auto it = _memPoolsData[pool].find(address); it != _memPoolsData[pool].end())
        {
            _memPoolsData[pool].erase(it);
        }

        _memPoolsData[pool].insert(p);
        return p;
    }

    AmVoidPtr MemoryManager::Realign(
        MemoryPoolKind pool, AmVoidPtr address, AmSize size, AmUInt32 alignment, const char* file, AmUInt32 line)
    {
        if (address == nullptr)
        {
#if !defined(AM_NO_MEMORY_STATS)
            _memPoolsStats[pool].maxMemoryUsed.fetch_add(size, std::memory_order_relaxed);
            _memPoolsStats[pool].allocCount.fetch_add(1, std::memory_order_relaxed);
#endif
        }

        if (_config.alignedRealloc != nullptr)
        {
            return _config.alignedRealloc(pool, address, size, alignment);
        }

        AmVoidPtr p = _sm_realloc(static_cast<sm_allocator>(_memAllocator), address, size, alignment);
        if (const auto it = _memPoolsData[pool].find(address); it != _memPoolsData[pool].end())
        {
            _memPoolsData[pool].erase(it);
        }

        _memPoolsData[pool].insert(p);
        return p;
    }

    void MemoryManager::Free(MemoryPoolKind pool, AmVoidPtr address)
    {
#if !defined(AM_NO_MEMORY_STATS)
        _memPoolsStats[pool].freeCount.fetch_add(1, std::memory_order_relaxed);
#endif

        if (_config.free != nullptr)
        {
            _config.free(pool, address);
        }
        else
        {
            _sm_free(static_cast<sm_allocator>(_memAllocator), address);
            if (const auto it = _memPoolsData[pool].find(address); it != _memPoolsData[pool].end())
            {
                _memPoolsData[pool].erase(it);
            }
        }
    }

    AmSize MemoryManager::TotalReservedMemorySize() const
    {
        if (_config.totalReservedMemorySize != nullptr)
        {
            return _config.totalReservedMemorySize();
        }

        AmSize total = 0;
        for (auto&& pool : _memPoolsData)
        {
            for (auto&& p : pool.second)
            {
                total += _sm_msize(static_cast<sm_allocator>(_memAllocator), p);
            }
        }

        return total;
    }

    AmSize MemoryManager::SizeOf(MemoryPoolKind pool, AmVoidPtr address) const
    {
        if (_config.sizeOf != nullptr)
        {
            return _config.sizeOf(pool, address);
        }

        return _sm_msize(static_cast<sm_allocator>(_memAllocator), address);
    }

#if !defined(AM_NO_MEMORY_STATS)
    const MemoryPoolStats& MemoryManager::GetStats(MemoryPoolKind pool) const
    {
        return _memPoolsStats.at(pool);
    }
#endif

    ScopedMemoryAllocation::ScopedMemoryAllocation(MemoryPoolKind pool, AmSize size, const char* file, AmUInt32 line)
    {
        _pool = pool;
        _address = amMemory->Malloc(_pool, size, file, line);
    }

    ScopedMemoryAllocation::ScopedMemoryAllocation(
        MemoryPoolKind pool, AmSize size, AmUInt32 alignment, const char* file, AmUInt32 line)
    {
        _pool = pool;
        _address = amMemory->Malign(_pool, size, alignment, file, line);
    }

    ScopedMemoryAllocation::~ScopedMemoryAllocation()
    {
        if (_address == nullptr)
            return;

        ampoolfree(_pool, _address);
        _address = nullptr;
    }
} // namespace SparkyStudios::Audio::Amplitude
