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

#ifndef SS_AMPLITUDE_AUDIO_EFFECT_PROCESSOR_H
#define SS_AMPLITUDE_AUDIO_EFFECT_PROCESSOR_H

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

namespace SparkyStudios::Audio::Amplitude
{
    [[maybe_unused]] static class EffectProcessor final : public SoundProcessor
    {
    public:
        EffectProcessor()
            : SoundProcessor("EffectProcessor")
        {}

        void Process(
            AmInt16Buffer out,
            AmInt16Buffer in,
            AmUInt64 frames,
            AmUInt64 bufferSize,
            AmUInt16 channels,
            AmUInt32 sampleRate,
            SoundInstance* sound) override
        {
            const EffectInstance* effect = sound->GetEffect();
            if (effect == nullptr)
            {
                return;
            }

            effect->GetFilter()->Process(out, frames, bufferSize, channels, sampleRate);
        }

        void ProcessInterleaved(
            AmInt16Buffer out,
            AmInt16Buffer in,
            AmUInt64 frames,
            AmUInt64 bufferSize,
            AmUInt16 channels,
            AmUInt32 sampleRate,
            SoundInstance* sound) override
        {
            const EffectInstance* effect = sound->GetEffect();
            if (effect == nullptr)
            {
                return;
            }

            effect->GetFilter()->ProcessInterleaved(out, frames, bufferSize, channels, sampleRate);
        }
    } gEffectProcessor; // NOLINT(cert-err58-cpp)
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_EFFECT_PROCESSOR_H
