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
    class EffectProcessorInstance final : public SoundProcessorInstance
    {
    public:
        void Process(
            AmAudioSampleBuffer out,
            AmConstAudioSampleBuffer in,
            AmUInt64 frames,
            AmSize bufferSize,
            AmUInt16 channels,
            AmUInt32 sampleRate,
            SoundInstance* sound) override
        {
            const EffectInstance* effect = sound->GetEffect();

            if (out != in)
                std::memcpy(out, in, bufferSize);

            if (effect == nullptr)
                return;

            effect->GetFilter()->Process(out, frames, bufferSize, channels, sampleRate);
        }

        void ProcessInterleaved(
            AmAudioSampleBuffer out,
            AmConstAudioSampleBuffer in,
            AmUInt64 frames,
            AmSize bufferSize,
            AmUInt16 channels,
            AmUInt32 sampleRate,
            SoundInstance* sound) override
        {
            const EffectInstance* effect = sound->GetEffect();

            if (out != in)
                std::memcpy(out, in, bufferSize);

            if (effect == nullptr)
                return;

            effect->GetFilter()->ProcessInterleaved(out, frames, bufferSize, channels, sampleRate);
        }
    };

    [[maybe_unused]] static class EffectProcessor final : public SoundProcessor
    {
    public:
        EffectProcessor()
            : SoundProcessor("EffectProcessor")
        {}

        SoundProcessorInstance* CreateInstance() override
        {
            return ampoolnew(MemoryPoolKind::Filtering, EffectProcessorInstance);
        }

        void DestroyInstance(SoundProcessorInstance* instance) override
        {
            ampooldelete(MemoryPoolKind::Filtering, EffectProcessorInstance, (EffectProcessorInstance*)instance);
        }
    } gEffectProcessor; // NOLINT(cert-err58-cpp)
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_EFFECT_PROCESSOR_H
