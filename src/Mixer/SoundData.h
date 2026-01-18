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

#ifndef _AM_IMPLEMENTATION_MIXER_SOUND_DATA_H
#define _AM_IMPLEMENTATION_MIXER_SOUND_DATA_H

#include <SparkyStudios/Audio/Amplitude/Core/AudioBuffer.h>
#include <SparkyStudios/Audio/Amplitude/Core/Common.h>
#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>

#include <Utils/Utils.h>

namespace SparkyStudios::Audio::Amplitude
{
    class SoundInstance;

    struct SoundChunk
    {
        AmUInt64 length;
        AmUInt64 frames;
        AmSize size;

        AudioBuffer* buffer;

        eMemoryPoolKind memoryPool;

        static SoundChunk* CreateChunk(AmUInt64 frames, AmUInt16 channels, eMemoryPoolKind pool = eMemoryPoolKind_SoundData);
        static void DestroyChunk(SoundChunk* chunk);

        ~SoundChunk();
    };

    /**
     * @brief Pool for reusing SoundChunk objects.
     *
     * This pool is optimized for usage in Amplimix layers, but can be used anywhere else. The pool grows
     * lazily up to kMaxChunksPerPool and reuses chunks with matching or larger capacity.
     */
    struct SoundChunkPool
    {
        static constexpr AmSize kMaxChunksPerPool = 8;

        struct PooledChunk
        {
            SoundChunk* chunk;
            bool inUse;
        };

        PooledChunk chunks[kMaxChunksPerPool];
        AmUInt32 allocated;
        eMemoryPoolKind memoryPool;

        SoundChunkPool();

        /**
         * @brief Acquires a chunk from the pool or allocates a new one.
         *
         * @param[in] frames Number of frames needed.
         * @param[in] channels Number of channels needed.
         * @return A SoundChunk pointer (never null).
         */
        SoundChunk* Acquire(AmUInt64 frames, AmUInt16 channels);

        /**
         * @brief Releases a chunk back to the pool.
         *
         * @param[in] chunk The chunk to release (may be null).
         */
        void Release(SoundChunk* chunk);

        /**
         * @brief Destroys all pooled chunks and resets the pool.
         */
        void Reset();

        ~SoundChunkPool();
    };

    struct SoundData
    {
        SoundData();

        SoundChunk* chunk;
        AmUInt64 length;
        AmUniquePtr<SoundInstance, eMemoryPoolKind_Amplimix> sound;
        SoundFormat format;
        bool stream;

        static SoundData* CreateMusic(const SoundFormat& format, SoundChunk* chunk, AmUInt64 frames, SoundInstance* soundInstance);
        static SoundData* CreateSound(const SoundFormat& format, SoundChunk* chunk, AmUInt64 frames, SoundInstance* soundInstance);

        static void Destroy(SoundData* soundData, bool destroyChunk = true);
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_MIXER_SOUND_DATA_H
