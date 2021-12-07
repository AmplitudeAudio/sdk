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

#include <Mixer/Mixer.h>
#include <Mixer/SoundData.h>

namespace SparkyStudios::Audio::Amplitude
{
    static SoundData* CreateSoundData(const SoundFormat& format, SoundChunk* chunk, AmVoidPtr userData, AmUInt64 frames, bool stream)
    {
        if (format.GetNumChannels() < 1 || format.GetNumChannels() > 2 || frames < 1)
            return nullptr;

        auto* sound = new SoundData();

        sound->chunk = chunk;
        sound->length = frames;
        sound->userData = userData;
        sound->format = format;
        sound->stream = stream;

        return sound;
    }

    SoundChunk* SoundChunk::CreateChunk(AmUInt64 frames, AmUInt16 channels)
    {
#if defined(AM_SSE_INTRINSICS)
        AmUInt64 alignedLength = AM_VALUE_ALIGN(frames * channels, Vc::int16_v::Size);
#else
        AmUInt64 alignedLength = frames * channels;
#endif // AM_SSE_INTRINSICS

        auto* chunk = new SoundChunk();

        chunk->frames = frames;
        chunk->length = alignedLength;
        chunk->buffer = static_cast<AudioBuffer>(amMemory->Malign(MemoryPoolKind::SoundData, alignedLength * sizeof(AmInt16), AM_SIMD_ALIGNMENT));

        return chunk;
    }

    void SoundChunk::DestroyChunk(SoundChunk* chunk)
    {
        // Mixer::OnSoundDestroyed(chunk);
        amMemory->Free(MemoryPoolKind::SoundData, chunk->buffer);
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
