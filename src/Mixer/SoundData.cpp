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

#include <SparkyStudios/Audio/Amplitude/Core/Log.h>
#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>

#include <Mixer/Mixer.h>
#include <Mixer/SoundData.h>

namespace SparkyStudios::Audio::Amplitude
{
    static SoundData* CreateSoundData(
        const SoundFormat& format, SoundChunk* chunk, SoundInstance* soundInstance, AmUInt64 frames, bool stream)
    {
        if (format.GetNumChannels() < 1 || format.GetNumChannels() > 2 || frames < 1)
            return nullptr;

        auto* sound = ampoolnew(MemoryPoolKind::SoundData, SoundData);

        sound->chunk = chunk;
        sound->length = frames;
        sound->sound.reset(soundInstance);
        sound->format = format;
        sound->stream = stream;

        return sound;
    }

    SoundChunk* SoundChunk::CreateChunk(AmUInt64 frames, AmUInt16 channels, MemoryPoolKind pool)
    {
#if defined(AM_SIMD_INTRINSICS)
        const AmUInt64 alignedFrames = AM_VALUE_ALIGN(frames, AmAudioFrame::size);
#else
        const AmUInt64 alignedFrames = frames;
#endif // AM_SIMD_INTRINSICS
        const AmUInt64 alignedLength = alignedFrames * channels;

        auto* chunk = ampoolnew(MemoryPoolKind::SoundData, SoundChunk);

        chunk->frames = alignedFrames;
        chunk->length = alignedLength;
        chunk->size = alignedLength * sizeof(AmReal32);
#if defined(AM_SIMD_INTRINSICS)
        chunk->samplesPerVector = AmAudioFrame::size / channels;
#endif // AM_SIMD_INTRINSICS
        chunk->memoryPool = pool;
#if defined(AM_SIMD_INTRINSICS)
        chunk->buffer = static_cast<AmAudioFrameBuffer>(ampoolmalign(pool, chunk->size, AM_SIMD_ALIGNMENT));
#else
        chunk->buffer = static_cast<AmAudioFrameBuffer>(ampoolmalloc(pool, chunk->size));
#endif // AM_SIMD_INTRINSICS

        if (chunk->buffer == nullptr)
        {
            CallLogFunc("[ERROR] Failed to allocate memory for sound chunk.");

            ampooldelete(MemoryPoolKind::SoundData, SoundChunk, chunk);
            return nullptr;
        }

        std::memset(chunk->buffer, 0, chunk->size);

        return chunk;
    }

    void SoundChunk::DestroyChunk(SoundChunk* chunk)
    {
        ampooldelete(MemoryPoolKind::SoundData, SoundChunk, chunk);
    }

    SoundChunk::~SoundChunk()
    {
        if (buffer == nullptr)
            return;

        ampoolfree(memoryPool, buffer);
        buffer = nullptr;
    }

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

        ampooldelete(MemoryPoolKind::SoundData, SoundData, soundData);
    }
} // namespace SparkyStudios::Audio::Amplitude
