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
#include <set>

namespace SparkyStudios::Audio::Amplitude
{
    static MemoryManager* gMemManager = nullptr;

    static sm_allocator gMemAllocator = nullptr;

    static std::map<MemoryPoolKind, std::set<AmVoidPtr>> gMemPoolsData = {};

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

    void MemoryManager::Init(const MemoryManagerConfig& config)
    {
        if (gMemManager == nullptr)
        {
            gMemManager = new MemoryManager(config);
        }
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
            gMemAllocator = _sm_allocator_create(config.bucketsCount, config.bucketsSizeInBytes);
        }
        else
        {
            AMPLITUDE_ASSERT(
                config.realloc != nullptr && config.free != nullptr && config.alignedMalloc != nullptr &&
                config.alignedRealloc != nullptr && config.sizeOf != nullptr);
            gMemAllocator = nullptr;
        }
    }

    MemoryManager::~MemoryManager()
    {
        if (_config.malloc == nullptr)
        {
            _sm_allocator_destroy(gMemAllocator);
        }
    }

    AmVoidPtr MemoryManager::Malloc(MemoryPoolKind pool, AmSize size)
    {
        if (_config.malloc != nullptr)
        {
            return _config.malloc(pool, size);
        }
        else
        {
            AmVoidPtr p = _sm_malloc(gMemAllocator, size, 0);
            gMemPoolsData[pool].insert(p);
            return p;
        }
    }

    AmVoidPtr MemoryManager::Malign(MemoryPoolKind pool, AmSize size, AmUInt32 alignment)
    {
        if (_config.alignedMalloc != nullptr)
        {
            return _config.alignedMalloc(pool, size, alignment);
        }
        else
        {
            AmVoidPtr p = _sm_malloc(gMemAllocator, size, alignment);
            gMemPoolsData[pool].insert(p);
            return p;
        }
    }

    AmVoidPtr MemoryManager::Realloc(MemoryPoolKind pool, AmVoidPtr address, AmSize size)
    {
        if (_config.realloc != nullptr)
        {
            return _config.realloc(pool, address, size);
        }
        else
        {
            AmVoidPtr p = _sm_realloc(gMemAllocator, address, size, 0);
            if (auto it = gMemPoolsData[pool].find(address); it != gMemPoolsData[pool].end())
            {
                gMemPoolsData[pool].erase(it);
            }
            gMemPoolsData[pool].insert(p);
            return p;
        }
    }

    AmVoidPtr MemoryManager::Realign(MemoryPoolKind pool, AmVoidPtr address, AmSize size, AmUInt32 alignment)
    {
        if (_config.alignedRealloc != nullptr)
        {
            return _config.alignedRealloc(pool, address, size, alignment);
        }
        else
        {
            AmVoidPtr p = _sm_realloc(gMemAllocator, address, size, alignment);
            if (auto it = gMemPoolsData[pool].find(address); it != gMemPoolsData[pool].end())
            {
                gMemPoolsData[pool].erase(it);
            }
            gMemPoolsData[pool].insert(p);
            return p;
        }
    }

    void MemoryManager::Free(MemoryPoolKind pool, AmVoidPtr address)
    {
        if (_config.free != nullptr)
        {
            _config.free(pool, address);
        }
        else
        {
            _sm_free(gMemAllocator, address);
            if (auto it = gMemPoolsData[pool].find(address); it != gMemPoolsData[pool].end())
            {
                gMemPoolsData[pool].erase(it);
            }
        }
    }

    AmSize MemoryManager::TotalReservedMemorySize()
    {
        if (_config.totalReservedMemorySize != nullptr)
        {
            return _config.totalReservedMemorySize();
        }
        else
        {
            AmSize total = 0;
            for (auto&& pool : gMemPoolsData)
            {
                for (auto&& p : pool.second)
                {
                    total += _sm_msize(gMemAllocator, p);
                }
            }

            return total;
        }
    }

    AmSize MemoryManager::SizeOf(MemoryPoolKind pool, AmVoidPtr address)
    {
        if (_config.sizeOf != nullptr)
        {
            return _config.sizeOf(pool, address);
        }
        else
        {
            return _sm_msize(gMemAllocator, address);
        }
    }
} // namespace SparkyStudios::Audio::Amplitude
