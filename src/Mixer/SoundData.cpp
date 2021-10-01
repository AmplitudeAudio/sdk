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

#include <Mixer/Mixer.h>
#include <Mixer/SoundData.h>

#include <Utils/SmMalloc/smmalloc.h>

namespace SparkyStudios::Audio::Amplitude
{
    static sm_allocator soundChunkAllocator = _sm_allocator_create(1, (16 * 1024 * 1024));

    static SoundData* CreateSoundData(const SoundFormat& format, SoundChunk* chunk, AmVoidPtr userData, AmInt32 length, bool stream)
    {
        if (format.GetNumChannels() < 1 || format.GetNumChannels() > 2 || length < 1)
            return nullptr;

        AmInt32 alignedLength = AM_VALUE_ALIGN(length, 4);
        auto* sound = new SoundData();

        sound->chunk = chunk;
        sound->length = alignedLength;
        sound->userData = userData;
        sound->format = format;
        sound->stream = stream;

        return sound;
    }

    SoundChunk* SoundChunk::CreateChunk(AmInt32 frames, AmInt32 channels)
    {
        AmInt32 alignedLength = AM_VALUE_ALIGN(frames * channels, 4);

        auto* chunk = new SoundChunk();

        chunk->frames = frames;
        chunk->length = alignedLength;
        chunk->buffer = (AudioBuffer)_sm_malloc(soundChunkAllocator, alignedLength * sizeof(AmInt16), AM_SIMD_ALIGNMENT);

        return chunk;
    }

    void SoundChunk::DestroyChunk(SoundChunk* chunk)
    {
        // Mixer::OnSoundDestroyed(chunk);
        _sm_free(soundChunkAllocator, chunk->buffer);
    }

    SoundData* SoundData::CreateMusic(const SoundFormat& format, SoundChunk* chunk, AmVoidPtr userData)
    {
        return CreateSoundData(format, chunk, userData, format.GetFramesCount(), true);
    }

    SoundData* SoundData::CreateSound(const SoundFormat& format, SoundChunk* chunk, AmVoidPtr userData)
    {
        return CreateSoundData(format, chunk, userData, format.GetFramesCount(), false);
    }

    void SoundData::Destroy()
    {
        SoundChunk::DestroyChunk(chunk);
    }
} // namespace SparkyStudios::Audio::Amplitude
