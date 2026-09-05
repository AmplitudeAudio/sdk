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
#include <SparkyStudios/Audio/Amplitude/IO/Log.h>

#include <Mixer/SoundData.h>
#include <Sound/Sound.h>

namespace SparkyStudios::Audio::Amplitude
{
    static SoundData* CreateSoundData(
        const SoundFormat& format, SoundChunk* chunk, SoundInstance* soundInstance, AmUInt64 frames, bool stream)
    {
        if (format.GetNumChannels() < 1 || format.GetNumChannels() > 2 || frames < 1)
            return nullptr;

        auto* sound = ampoolnew(eMemoryPoolKind_SoundData, SoundData);

        sound->chunk = chunk;
        sound->length = frames;
        sound->sound.reset(soundInstance);
        sound->format = format;
        sound->stream = stream;

        return sound;
    }

    SoundChunk* SoundChunk::CreateChunk(AmUInt64 frames, AmUInt16 channels, eMemoryPoolKind pool)
    {
#if defined(AM_SIMD_INTRINSICS)
        const AmUInt64 alignedFrames = AM_VALUE_ALIGN(frames, GetSimdBlockSize());
#else
        const AmUInt64 alignedFrames = frames;
#endif // AM_SIMD_INTRINSICS
        const AmUInt64 alignedLength = alignedFrames * channels;

        auto* chunk = ampoolnew(eMemoryPoolKind_SoundData, SoundChunk);

        chunk->frames = alignedFrames;
        chunk->length = alignedLength;
        chunk->size = alignedLength * sizeof(AmReal32);
        chunk->memoryPool = pool;
        chunk->buffer = ampoolnew(pool, AudioBuffer, chunk->frames, channels);

        return chunk;
    }

    void SoundChunk::DestroyChunk(SoundChunk* chunk)
    {
        ampooldelete(eMemoryPoolKind_SoundData, SoundChunk, chunk);
    }

    SoundChunk::~SoundChunk()
    {
        if (buffer == nullptr)
            return;

        buffer->Clear();
        ampooldelete(memoryPool, AudioBuffer, buffer);
        buffer = nullptr;
    }

    SoundChunkPool::SoundChunkPool()
        : allocated(0)
        , memoryPool(eMemoryPoolKind_Amplimix)
    {
        for (AmSize i = 0; i < kMaxChunksPerPool; ++i)
        {
            chunks[i].chunk = nullptr;
            chunks[i].inUse = false;
        }
    }

    SoundChunk* SoundChunkPool::Acquire(AmUInt64 frames, AmUInt16 channels, bool clearOnAcquire)
    {
        // Search for reusable chunk with matching or larger capacity
        for (AmSize i = 0; i < allocated; ++i)
        {
            if (!chunks[i].inUse && chunks[i].chunk != nullptr)
            {
                SoundChunk* chunk = chunks[i].chunk;
                const AmUInt16 chunkChannels = chunk->buffer->GetChannelCount();

                // Check if chunk has sufficient capacity
                if (chunk->frames >= frames && chunkChannels == channels)
                {
                    chunks[i].inUse = true;
                    if (clearOnAcquire)
                        chunk->buffer->Clear();
                    return chunk;
                }
            }
        }

        SoundChunk* newChunk = SoundChunk::CreateChunk(frames, channels, memoryPool);

        if (allocated < kMaxChunksPerPool)
        {
            chunks[allocated].chunk = newChunk;
            chunks[allocated].inUse = true;
            allocated++;
        }
        else
        {
            // Pool full - this chunk will be destroyed on Release
            amLogWarning("SoundChunkPool exhausted (%zu chunks), falling back to direct allocation", kMaxChunksPerPool);
        }

        return newChunk;
    }

    void SoundChunkPool::PreWarm(AmUInt64 inFrames, AmUInt16 inChannels, AmUInt64 outFrames)
    {
        SoundChunk* in = Acquire(inFrames, inChannels, false);
        SoundChunk* transient = Acquire(outFrames, static_cast<AmUInt16>(kAmMonoChannelCount), false);
        SoundChunk* out = Acquire(outFrames, static_cast<AmUInt16>(kAmStereoChannelCount), false);

        Release(out);
        Release(transient);
        Release(in);
    }

    void SoundChunkPool::Release(SoundChunk* chunk)
    {
        if (chunk == nullptr)
            return;

        // Find chunk in pool and mark as available
        for (AmSize i = 0; i < allocated; ++i)
        {
            if (chunks[i].chunk == chunk)
            {
                chunks[i].inUse = false;
                return;
            }
        }

        // Not in pool (emergency allocation) - destroy it
        SoundChunk::DestroyChunk(chunk);
    }

    void SoundChunkPool::Reset()
    {
        for (AmSize i = 0; i < allocated; ++i)
        {
            if (chunks[i].chunk != nullptr)
            {
                SoundChunk::DestroyChunk(chunks[i].chunk);
                chunks[i].chunk = nullptr;
            }

            chunks[i].inUse = false;
        }

        allocated = 0;
    }

    SoundChunkPool::~SoundChunkPool()
    {
        Reset();
    }

    SoundData::SoundData()
        : chunk(nullptr)
        , length(0)
        , sound(nullptr)
        , format()
        , stream(false)
    {}

    SoundData* SoundData::CreateMusic(const SoundFormat& format, SoundChunk* chunk, AmUInt64 frames, SoundInstance* soundInstance)
    {
        return CreateSoundData(format, chunk, soundInstance, frames, true);
    }

    SoundData* SoundData::CreateSound(const SoundFormat& format, SoundChunk* chunk, AmUInt64 frames, SoundInstance* soundInstance)
    {
        return CreateSoundData(format, chunk, soundInstance, frames, false);
    }

    void SoundData::Destroy(SoundData* soundData, bool destroyChunk)
    {
        if (soundData == nullptr)
            return;

        if (destroyChunk)
            SoundChunk::DestroyChunk(soundData->chunk);

        ampooldelete(eMemoryPoolKind_SoundData, SoundData, soundData);
    }
} // namespace SparkyStudios::Audio::Amplitude
