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

        AmAudioFrameBuffer buffer;

#if defined(AM_SIMD_INTRINSICS)
        AmUInt64 samplesPerVector;
#endif // AM_SIMD_INTRINSICS

        MemoryPoolKind memoryPool;

        static SoundChunk* CreateChunk(AmUInt64 frames, AmUInt16 channels, MemoryPoolKind pool = MemoryPoolKind::SoundData);
        static void DestroyChunk(SoundChunk* chunk);

        ~SoundChunk();
    };

    struct SoundData
    {
        SoundChunk* chunk = nullptr;
        AmUInt64 length = 0;
        AmUniquePtr<MemoryPoolKind::Engine, SoundInstance> sound = nullptr;
        SoundFormat format{};
        bool stream = false;

        static SoundData* CreateMusic(const SoundFormat& format, SoundChunk* chunk, AmUInt64 frames, SoundInstance* soundInstance);
        static SoundData* CreateSound(const SoundFormat& format, SoundChunk* chunk, AmUInt64 frames, SoundInstance* soundInstance);

        static void Destroy(SoundData* soundData, bool destroyChunk = true);
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_SOUNDDATA_H
