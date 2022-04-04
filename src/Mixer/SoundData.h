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

#ifndef SS_AMPLITUDE_AUDIO_SOUNDDATA_H
#define SS_AMPLITUDE_AUDIO_SOUNDDATA_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>
#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>

#include <Utils/Utils.h>

namespace SparkyStudios::Audio::Amplitude
{
    struct SoundChunk
    {
        AmUInt64 length;
        AmUInt64 frames;
        AmSize size;

        AudioBuffer buffer;

#if defined(AM_SSE_INTRINSICS)
        AmUInt64 samplesPerVector;
#endif // AM_SSE_INTRINSICS

        MemoryPoolKind memoryPool;

        static SoundChunk* CreateChunk(AmUInt64 frames, AmUInt16 channels, MemoryPoolKind pool = MemoryPoolKind::SoundData);
        static void DestroyChunk(SoundChunk* chunk);
    };

    struct SoundData
    {
        SoundChunk* chunk;
        AmUInt64 length;
        AmVoidPtr userData;
        SoundFormat format;
        bool stream;

        static SoundData* CreateMusic(const SoundFormat& format, SoundChunk* chunk, AmUInt64 frames, AmVoidPtr userData);
        static SoundData* CreateSound(const SoundFormat& format, SoundChunk* chunk, AmUInt64 frames, AmVoidPtr userData);

        void Destroy(bool destroyChunk = true) const;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_SOUNDDATA_H
