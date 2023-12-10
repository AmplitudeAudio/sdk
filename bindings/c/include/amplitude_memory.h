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

#ifndef SS_AMPLITUDE_MEMORY_H
#define SS_AMPLITUDE_MEMORY_H

#include "amplitude_common.h"

#ifndef AM_NO_MEMORY_STATS

struct am_memory_pool_stats; // Opaque type for the Engine class.
typedef struct am_memory_pool_stats am_memory_pool_stats;
typedef am_memory_pool_stats* am_memory_pool_stats_handle;

#endif

/**
 * @brief Available memory pools.
 */
typedef enum am_memory_pool_kind
    : am_uint8
{
    /**
     * @brief Amplitude Engine allocations.
     */
    am_memory_pool_kind_engine,

    /**
     * @brief Amplimix allocations.
     */
    am_memory_pool_kind_amplimix,

    /**
     * @brief Sound data and streams.
     */
    am_memory_pool_kind_sound_data,

    /**
     * @brief Filters related allocations.
     */
    am_memory_pool_kind_filtering,

    /**
     * @brief Encoding/Decoding allocations.
     *
     */
    am_memory_pool_kind_codec,

    /**
     * @brief I/O and filesystem related allocations.
     */
    am_memory_pool_kind_io,

    /**
     * @brief Default allocations pool. Use this when the allocated memory pool is not available.
     * @note amnew use this pool to allocate memory from the memory manager.
     */
    am_memory_pool_kind_default,

    /**
     * @brief The total number of memory pools.
     */
    am_memory_pool_kind_max,
} am_memory_pool_kind;

typedef void* (*am_memory_malloc_callback)(am_memory_pool_kind pool, am_size size);

typedef void* (*am_memory_realloc_callback)(am_memory_pool_kind pool, void* address, am_size size);

typedef void* (*am_memory_malloc_aligned_callback)(am_memory_pool_kind pool, am_size size, am_uint32 alignment);

typedef void* (*am_memory_realloc_aligned_callback)(am_memory_pool_kind pool, void* address, am_size size, am_uint32 alignment);

typedef void (*am_memory_free_callback)(am_memory_pool_kind pool, void* address);

typedef am_size (*am_memory_total_reserved_memory_size_callback)();

typedef am_size (*am_memory_size_of_callback)(am_memory_pool_kind pool, const void* address);

/**
 * @brief Configures the memory management system.
 */
typedef struct am_memory_manager_config
{
    /**
     * @brief Memory allocation callback.
     */
    am_memory_malloc_callback malloc;

    /**
     * @brief Memory reallocation callback.
     */
    am_memory_realloc_callback realloc;

    /**
     * @brief Aligned memory allocation callback.
     */
    am_memory_malloc_aligned_callback malign;

    /**
     * @brief Aligned memory reallocation callback.
     */
    am_memory_realloc_aligned_callback realign;

    /**
     * @brief Memory release callback.
     */
    am_memory_free_callback free;

    /**
     * @brief Callback to get the total size of the memory allocated across memory pools
     *
     */
    am_memory_total_reserved_memory_size_callback total_reserved_memory_size;

    /**
     * @brief Callback to get the total size of memory for a specific pool.
     */
    am_memory_size_of_callback size_of;
} am_memory_manager_config;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes an empty memory manager configuration.
 *
 * @return An empty memory manager configuration.
 */
am_memory_manager_config am_memory_manager_config_init();

/**
 * @brief Initializes the memory manager.
 *
 * @note This should be done prior to any call of GetInstance().
 */
void am_memory_manager_initialize(const am_memory_manager_config* config);

/**
 * @brief Unloads the memory manager.
 */
void am_memory_manager_deinitialize();

/**
 * @brief Checks whether the memory manager is initialized.
 *
 * @return Whether the memory manager is initialized.
 */
am_bool am_memeory_manager_is_initialized();

/**
 * @brief Allocates a block of memory with the given size in the given pool.
 *
 * @param pool The memory pool to allocate from.
 * @param size The size of the block to allocate.
 * @param file The file in which the allocation was made.
 * @param line The line in which the allocation was made.
 *
 * @return A pointer to the allocated block.
 */
void* am_memory_manager_malloc(am_memory_pool_kind pool, am_size size, const char* file, am_uint32 line);

/**
 * @brief Allocates a block of memory with the given size and the given alignment,
 * in the given pool.
 *
 * @param pool The memory pool to allocate from.
 * @param size The size of the block to allocate.
 * @param alignment The alignment of the block to allocate.
 * @param file The file in which the allocation was made.
 * @param line The line in which the allocation was made.
 *
 * @return A pointer to the allocated block.
 */
void* am_memory_manager_malign(am_memory_pool_kind pool, am_size size, am_uint32 alignment, const char* file, am_uint32 line);

/**
 * @brief Updates the size of a previously allocated memory.
 *
 * @param pool The memory pool to update.
 * @param address The address of the memory to update.
 * @param size The new size of the memory.
 * @param file The file in which the allocation was made.
 * @param line The line in which the allocation was made.
 *
 * @return A pointer to the allocated block. Maybe equal to address if the original pointer had enough memory.
 */
void* am_memory_manager_realloc(am_memory_pool_kind pool, void* address, am_size size, const char* file, am_uint32 line);

/**
 * @brief Updates the size of a previously allocated aligned memory.
 *
 * @param pool The memory pool to update.
 * @param address The address of the aligned memory to update.
 * @param size The new size of the aligned memory.
 * @param alignment The new alignment of the aligned memory.
 * @param file The file in which the allocation was made.
 * @param line The line in which the allocation was made.
 *
 * @return A pointer to the allocated block. Maybe equal to address if the original pointer had enough memory.
 */
void* am_memory_manager_realign(
    am_memory_pool_kind pool, void* address, am_size size, am_uint32 alignment, const char* file, am_uint32 line);

/**
 * @brief Releases an allocated memory block.
 *
 * @param pool The memory pool to release from.
 * @param address The address of the memory to release.
 */
void am_memory_manager_free(am_memory_pool_kind pool, void* address);

/**
 * @brief Gets the total allocated size.
 *
 * @return The total currently allocated size.
 */
am_size am_memory_manager_total_reserved_memory_size();

/**
 * @brief Gets the size of the given memory block.
 *
 * @param pool The memory pool to get the size from.
 * @param address The address of the memory block.
 *
 * @return The size of the given memory block.
 */
am_size am_memory_manager_size_of(am_memory_pool_kind pool, const void* address);

#ifndef AM_NO_MEMORY_STATS
/**
 * @brief Gets the name of the given memory pool.
 *
 * @param pool The memory pool to get the name for.
 *
 * @return The name of the memory pool.
 */
const char* am_memory_manager_get_memory_pool_name(am_memory_pool_kind pool);

/**
 * @brief Returns the memory allocation statistics for the given pool.
 *
 * @param pool The pool to get the statistics for.
 */
am_memory_pool_stats_handle am_memory_manager_get_stats(am_memory_pool_kind pool);

/**
 * @brief Inspect the memory manager for memory leaks.
 *
 * @note This function is most useful after the engine has been deinitialized. Calling it before may just
 * report a lot of false positives (allocated memories still in use).
 *
 * @return A string containing a report for the detected memory leaks.
 */
const char* am_memory_manager_inspect_memory_leaks();
#endif

#ifdef __cplusplus
}
#endif

#endif // SS_AMPLITUDE_MEMORY_H
