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

#pragma once

#ifndef SS_AMPLITUDE_AUDIO_MEMORY_H
#define SS_AMPLITUDE_AUDIO_MEMORY_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

/**
 * @brief Shortcut access to the Amplitude's memory manager instance.
 */
#define amMemory SparkyStudios::Audio::Amplitude::MemoryManager::GetInstance()

/**
 * @brief Allocates memory for a new object using the memory manager.
 *
 * This will create a new memory allocation in the Default pool. The allocated
 * memory will be freed when the object is destroyed using @a amdelete.
 *
 * @see amdelete
 */
#define amnew(_type_, ...)                                                                                                                 \
    new (amMemory->Malign(SparkyStudios::Audio::Amplitude::MemoryPoolKind::Default, sizeof(_type_), alignof(_type_))) _type_(__VA_ARGS__)

/**
 * @brief Deallocates a memory allocated with @a amnew.
 *
 * This will call the object's destructor before the memory is freed.
 *
 * @see amnew
 */
#define amdelete(_type_, _ptr_)                                                                                                            \
    {                                                                                                                                      \
        if (_ptr_ != nullptr)                                                                                                              \
        {                                                                                                                                  \
            (_ptr_)->~_type_();                                                                                                            \
            amMemory->Free(SparkyStudios::Audio::Amplitude::MemoryPoolKind::Default, (_ptr_));                                             \
        }                                                                                                                                  \
    }

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief Available memory pools.
     */
    enum class MemoryPoolKind : AmUInt8
    {
        /**
         * @brief Amplitude Engine allocations.
         */
        Engine,

        /**
         * @brief Amplimix allocations.
         */
        Amplimix,

        /**
         * @brief Sound data and streams.
         */
        SoundData,

        /**
         * @brief Filters related allocations.
         */
        Filtering,

        /**
         * @brief Encoding/Decoding allocations.
         *
         */
        Codec,

        /**
         * @brief Default allocations pool. Use this when the allocated memory pool is not available.
         * @note amneww use this pool to allocate memory from the memory manager.
         */
        Default,

        /**
         * @brief The total number of memory pools.
         */
        COUNT,
    };

    AM_CALLBACK(AmVoidPtr, AmMemoryMallocCallback)(MemoryPoolKind pool, AmSize size);

    AM_CALLBACK(AmVoidPtr, AmMemoryReallocCallback)(MemoryPoolKind pool, AmVoidPtr address, AmSize size);

    AM_CALLBACK(AmVoidPtr, AmMemoryMallocAlignedCallback)(MemoryPoolKind pool, AmSize size, AmUInt32 alignment);

    AM_CALLBACK(AmVoidPtr, AmMemoryReallocAlignedCallback)(MemoryPoolKind pool, AmVoidPtr address, AmSize size, AmUInt32 alignment);

    AM_CALLBACK(void, AmMemoryFreeCallback)(MemoryPoolKind pool, AmVoidPtr address);

    AM_CALLBACK(AmSize, AmMemoryTotalReservedMemorySizeCallback)();

    AM_CALLBACK(AmSize, AmMemorySizeOfCallback)(MemoryPoolKind pool, AmVoidPtr address);

    /**
     * @brief Configures the memory management system.
     */
    struct MemoryManagerConfig
    {
        /**
         * @brief Memory allocation callback. If not defined, the default malloc() function is used.
         */
        AmMemoryMallocCallback malloc;

        /**
         * @brief Memory reallocation callback. If not defined, the default realloc() function is used.
         */
        AmMemoryReallocCallback realloc;

        /**
         * @brief Aligned memory allocation callback. If not defined, the default platform specific
         * function is used. Check the AM_ALIGNED_ALLOC macro to know more.
         */
        AmMemoryMallocAlignedCallback alignedMalloc;

        /**
         * @brief Aligned memory reallocation callback. If not defined, the default platform specific
         * function is used. Check the AM_ALIGNED_REALLOC macro to know more.
         */
        AmMemoryReallocAlignedCallback alignedRealloc;

        /**
         * @brief Memory release callback. If not defined, the default free() function is used.
         */
        AmMemoryFreeCallback free;

        /**
         * @brief Callback to get the total size of the memory allocated accross memory pools
         *
         */
        AmMemoryTotalReservedMemorySizeCallback totalReservedMemorySize;

        /**
         * @brief Callback to get the total size of memory for a specific pool.
         */
        AmMemorySizeOfCallback sizeOf;

        /**
         * @brief Sets the number of buckets for the allocator.
         *
         * @remarks This is used only when allocation functions are not overriden,
         * to set up the internal memory allocator.
         */
        AmUInt32 bucketsCount;

        /**
         * @brief Sets the size of each buckets.
         *
         * @remarks This is used only when allocation functions are not overriden,
         * to set up the internal memory allocator.
         */
        AmSize bucketsSizeInBytes;

        /**
         * @brief Creates a new configuration set for the memory manager.
         */
        MemoryManagerConfig();
    };

#if !defined(AM_NO_MEMORY_STATS)
    /**
     * @brief Collects the statistics about the memory allocations
     * for a specific pool
     */
    struct MemoryPoolStats
    {
        /**
         * @brief The pool for which this statistics is for.
         */
        MemoryPoolKind pool;

        /**
         * @brief The maximum total memory used by this pool.
         */
        std::atomic<AmSize> maxMemoryUsed;

        /**
         * @brief The total count of allocations made on this pool.
         */
        std::atomic<AmUInt64> allocCount;

        /**
         * @brief The total count of frees made on this pool.
         */
        std::atomic<AmUInt64> freeCount;

        /**
         * @brief Default constructor.
         */
        MemoryPoolStats()
            : MemoryPoolStats(MemoryPoolKind::COUNT)
        {}

        /**
         * @brief Creates a new MemoryPoolStats object.
         *
         * @param pool The pool to get the statistics for.
         */
        explicit MemoryPoolStats(MemoryPoolKind pool);

        MemoryPoolStats& operator=(const MemoryPoolStats& other);
    };
#endif

    /**
     * @brief Manages memory allocations inside the engine.
     */
    class MemoryManager
    {
    public:
        /**
         * @brief Initializes the memory manager.
         *
         * @remarks This should be done prior to any call of GetInstance().
         */
        static void Initialize(const MemoryManagerConfig& config);

        /**
         * @brief Unloads the memory manager.
         */
        static void Deinitialize();

        /**
         * @brief Checks whether the memory manager is initialized.
         *
         * @return Whether the memory manager is initialized.
         */
        static bool IsInitialized();

        /**
         * @brief Gets the actual instance of the memory manager.
         */
        static MemoryManager* GetInstance();

        /**
         * @brief Allocates a block of memory with the given size in the given pool.
         */
        [[nodiscard]] AmVoidPtr Malloc(MemoryPoolKind pool, AmSize size) const;

        /**
         * @brief Allocates a block of memory with the given size and the given alignment,
         * in the given pool.
         */
        [[nodiscard]] AmVoidPtr Malign(MemoryPoolKind pool, AmSize size, AmUInt32 alignment) const;

        /**
         * @brief Updates the size of a previously allocated memory.
         */
        [[nodiscard]] AmVoidPtr Realloc(MemoryPoolKind pool, AmVoidPtr address, AmSize size) const;

        /**
         * @brief Updates the size of a previously allocated aligned memory.
         */
        [[nodiscard]] AmVoidPtr Realign(MemoryPoolKind pool, AmVoidPtr address, AmSize size, AmUInt32 alignment) const;

        /**
         * @brief Releases an allocated memory block.
         */
        void Free(MemoryPoolKind pool, AmVoidPtr address) const;

        /**
         * @brief Gets the total allocated size.
         */
        [[nodiscard]] AmSize TotalReservedMemorySize() const;

        /**
         * @brief Gets the size of the given memory block.
         */
        [[nodiscard]] AmSize SizeOf(MemoryPoolKind pool, AmVoidPtr address) const;

#if !defined(AM_NO_MEMORY_STATS)
        /**
         * @brief Returns the memory allocation statistics for the given pool.
         *
         * @param pool The pool to get the statistics for.
         */
        [[nodiscard]] const MemoryPoolStats& GetStats(MemoryPoolKind pool) const;
#endif

    private:
        explicit MemoryManager(MemoryManagerConfig config);
        ~MemoryManager();

        MemoryManagerConfig _config;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_MEMORY_H