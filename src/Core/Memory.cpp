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

#include <mimalloc.h>
#include <sstream>

namespace SparkyStudios::Audio::Amplitude
{
    AM_API_PUBLIC MemoryManager* gMemManager = nullptr;

#if !defined(AM_NO_MEMORY_STATS)
    AM_API_PRIVATE std::map<MemoryPoolKind, std::string> gMemoryPoolNames = {
        { MemoryPoolKind::Amplimix, "Amplimix" },   { MemoryPoolKind::Codec, "Codec" },         { MemoryPoolKind::Engine, "Engine" },
        { MemoryPoolKind::Filtering, "Filtering" }, { MemoryPoolKind::SoundData, "SoundData" }, { MemoryPoolKind::IO, "IO" },
        { MemoryPoolKind::Default, "Default" },
    };
#endif

    MemoryManagerConfig::MemoryManagerConfig()
        : malloc(nullptr)
        , realloc(nullptr)
        , alignedMalloc(nullptr)
        , alignedRealloc(nullptr)
        , free(nullptr)
        , totalReservedMemorySize(nullptr)
        , sizeOf(nullptr)
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

    MemoryManager::MemoryManager(const MemoryManagerConfig& config)
        : _config(config)
    {
        if (config.malloc != nullptr)
        {
            AMPLITUDE_ASSERT(
                config.realloc != nullptr && config.free != nullptr && config.alignedMalloc != nullptr &&
                config.alignedRealloc != nullptr && config.sizeOf != nullptr);
        }
    }

    MemoryManager::~MemoryManager()
    {
        if (_config.malloc == nullptr)
            mi_collect(true);
    }

    AmVoidPtr MemoryManager::Malloc(MemoryPoolKind pool, AmSize size, const char* file, AmUInt32 line)
    {
#if !defined(AM_NO_MEMORY_STATS)
        _memPoolsStats[pool].maxMemoryUsed.fetch_add(size, std::memory_order_relaxed);
        _memPoolsStats[pool].allocCount.fetch_add(1, std::memory_order_relaxed);
#endif

        AmVoidPtr ptr;

        if (_config.malloc != nullptr)
            ptr = _config.malloc(pool, size);
        else
            ptr = mi_malloc(size);

        _memAllocations.insert({ pool, ptr, SizeOf(pool, ptr), file, line });
        return ptr;
    }

    AmVoidPtr MemoryManager::Malign(MemoryPoolKind pool, AmSize size, AmUInt32 alignment, const char* file, AmUInt32 line)
    {
#if !defined(AM_NO_MEMORY_STATS)
        _memPoolsStats[pool].maxMemoryUsed.fetch_add(size, std::memory_order_relaxed);
        _memPoolsStats[pool].allocCount.fetch_add(1, std::memory_order_relaxed);
#endif

        AmVoidPtr ptr;

        if (_config.alignedMalloc != nullptr)
            ptr = _config.alignedMalloc(pool, size, alignment);
        else
            ptr = mi_malloc_aligned(size, alignment);

        _memAllocations.insert({ pool, ptr, SizeOf(pool, ptr), file, line });

        return ptr;
    }

    AmVoidPtr MemoryManager::Realloc(MemoryPoolKind pool, AmVoidPtr address, AmSize size, const char* file, AmUInt32 line)
    {
#if !defined(AM_NO_MEMORY_STATS)
        if (address == nullptr)
        {
            _memPoolsStats[pool].maxMemoryUsed.fetch_add(size, std::memory_order_relaxed);
            _memPoolsStats[pool].allocCount.fetch_add(1, std::memory_order_relaxed);
        }
#endif

        AmVoidPtr ptr;

        if (_config.realloc != nullptr)
            ptr = _config.realloc(pool, address, size);
        else
            ptr = mi_realloc(address, size);

        _memAllocations.insert({ pool, ptr, SizeOf(pool, ptr), file, line });

        return ptr;
    }

    AmVoidPtr MemoryManager::Realign(
        MemoryPoolKind pool, AmVoidPtr address, AmSize size, AmUInt32 alignment, const char* file, AmUInt32 line)
    {
#if !defined(AM_NO_MEMORY_STATS)
        if (address == nullptr)
        {
            _memPoolsStats[pool].maxMemoryUsed.fetch_add(size, std::memory_order_relaxed);
            _memPoolsStats[pool].allocCount.fetch_add(1, std::memory_order_relaxed);
        }
#endif

        AmVoidPtr ptr;

        if (_config.alignedRealloc != nullptr)
            ptr = _config.alignedRealloc(pool, address, size, alignment);
        else
            ptr = mi_realloc_aligned(address, size, alignment);

        _memAllocations.insert({ pool, ptr, SizeOf(pool, ptr), file, line });

        return ptr;
    }

    void MemoryManager::Free(MemoryPoolKind pool, AmVoidPtr address)
    {
#if !defined(AM_NO_MEMORY_STATS)
        _memPoolsStats[pool].freeCount.fetch_add(1, std::memory_order_relaxed);
#endif

        if (_config.free != nullptr)
            _config.free(pool, address);
        else
            mi_free(address);

        if (const auto it = _memAllocations.find({ pool, address }); it != _memAllocations.end())
            _memAllocations.erase(it);
    }

    AmSize MemoryManager::TotalReservedMemorySize() const
    {
        if (_config.totalReservedMemorySize != nullptr)
            return _config.totalReservedMemorySize();

        AmSize total = 0;
        for (const auto& allocation : _memAllocations)
            total += allocation.size;

        return total;
    }

    AmSize MemoryManager::SizeOf(MemoryPoolKind pool, AmConstVoidPtr address) const
    {
        if (_config.sizeOf != nullptr)
            return _config.sizeOf(pool, address);

        return mi_malloc_size(address);
    }

#if !defined(AM_NO_MEMORY_STATS)
    AmString MemoryManager::GetMemoryPoolName(const MemoryPoolKind pool)
    {
        return gMemoryPoolNames[pool];
    }

    const MemoryPoolStats& MemoryManager::GetStats(MemoryPoolKind pool) const
    {
        return _memPoolsStats.at(pool);
    }

    AmString MemoryManager::InspectMemoryLeaks() const
    {
        if (_memAllocations.empty())
            return "No memory leaks detected";

        std::stringstream ss;
        ss << "=== Memory leaks detected ===\n\n";

        for (auto&& allocation : _memAllocations)
        {
            ss << "Pool: " << gMemoryPoolNames[allocation.pool] << std::endl;
            ss << "  Address: " << allocation.address << std::endl;
            ss << "  Size: " << allocation.size << std::endl;
            ss << "  File: " << allocation.file << std::endl;
            ss << "  Line: " << allocation.line << std::endl << std::endl;
        }

        return ss.str();
    }
#endif

    ScopedMemoryAllocation::ScopedMemoryAllocation(MemoryPoolKind pool, AmSize size, const char* file, AmUInt32 line)
    {
        _pool = pool;
        _address = amMemory->Malloc(_pool, size, file, line);
    }

    ScopedMemoryAllocation::ScopedMemoryAllocation(MemoryPoolKind pool, AmSize size, AmUInt32 alignment, const char* file, AmUInt32 line)
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
