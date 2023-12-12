// Copyright (c) 2023-present Sparky Studios. All rights reserved.
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

#include <c/include/amplitude_memory.h>

#include "amplitude_internals.h"

extern "C" {
AM_API_PUBLIC am_memory_manager_config am_memory_manager_config_init()
{
    return { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
}

AM_API_PUBLIC void am_memory_manager_initialize(const am_memory_manager_config* config)
{
    MemoryManagerConfig c{};

    c.malloc = reinterpret_cast<AmMemoryMallocCallback>(config->malign);
    c.realloc = reinterpret_cast<AmMemoryReallocCallback>(config->realloc);
    c.alignedMalloc = reinterpret_cast<AmMemoryMallocAlignedCallback>(config->malign);
    c.alignedRealloc = reinterpret_cast<AmMemoryReallocAlignedCallback>(config->realign);
    c.free = reinterpret_cast<AmMemoryFreeCallback>(config->free);
    c.totalReservedMemorySize = static_cast<AmMemoryTotalReservedMemorySizeCallback>(config->total_reserved_memory_size);
    c.sizeOf = reinterpret_cast<AmMemorySizeOfCallback>(config->size_of);

    MemoryManager::Initialize(c);
}

AM_API_PUBLIC void am_memory_manager_deinitialize()
{
    MemoryManager::Deinitialize();
}

AM_API_PUBLIC am_bool am_memory_manager_is_initialized()
{
    return BOOL_TO_AM_BOOL(MemoryManager::IsInitialized());
}

AM_API_PUBLIC am_voidptr am_memory_manager_malloc(am_memory_pool_kind pool, am_size size, const char* file, am_uint32 line)
{
    return amMemory->Malloc(static_cast<MemoryPoolKind>(pool), size, file, line);
}

AM_API_PUBLIC am_voidptr am_memory_manager_malign(am_memory_pool_kind pool, am_size size, am_uint32 alignment, const char* file, am_uint32 line)
{
    return amMemory->Malign(static_cast<MemoryPoolKind>(pool), size, alignment, file, line);
}

AM_API_PUBLIC am_voidptr am_memory_manager_realloc(am_memory_pool_kind pool, am_voidptr address, am_size size, const char* file, am_uint32 line)
{
    return amMemory->Realloc(static_cast<MemoryPoolKind>(pool), address, size, file, line);
}

AM_API_PUBLIC am_voidptr am_memory_manager_realign(
    am_memory_pool_kind pool, am_voidptr address, am_size size, am_uint32 alignment, const char* file, am_uint32 line)
{
    return amMemory->Realign(static_cast<MemoryPoolKind>(pool), address, size, alignment, file, line);
}

AM_API_PUBLIC void am_memory_manager_free(am_memory_pool_kind pool, am_voidptr address)
{
    amMemory->Free(static_cast<MemoryPoolKind>(pool), address);
}

AM_API_PUBLIC am_size am_memory_manager_total_reserved_memory_size()
{
    return amMemory->TotalReservedMemorySize();
}

AM_API_PUBLIC am_size am_memory_manager_size_of(am_memory_pool_kind pool, const am_voidptr address)
{
    return amMemory->SizeOf(static_cast<MemoryPoolKind>(pool), address);
}

#ifndef AM_NO_MEMORY_STATS
AM_API_PUBLIC const char* am_memory_manager_get_memory_pool_name(am_memory_pool_kind pool)
{
    return MemoryManager::GetMemoryPoolName(static_cast<MemoryPoolKind>(pool)).c_str();
}

AM_API_PUBLIC am_memory_pool_stats_handle am_memory_manager_get_stats(am_memory_pool_kind pool)
{
    return nullptr;
}

AM_API_PUBLIC const char* am_memory_manager_inspect_memory_leaks()
{
    return amMemory->InspectMemoryLeaks().c_str();
}
#endif
}
