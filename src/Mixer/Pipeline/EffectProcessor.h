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

#ifndef _AM_IMPLEMENTATION_MIXER_PIPELINE_EFFECT_PROCESSOR_H
#define _AM_IMPLEMENTATION_MIXER_PIPELINE_EFFECT_PROCESSOR_H

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

namespace SparkyStudios::Audio::Amplitude
{
    class EffectProcessorInstance final : public SoundProcessorInstance
    {
    public:
        void Process(const AmplimixLayer* layer, const AudioBuffer& in, AudioBuffer& out) override
        {
            const EffectInstance* effect = layer->GetEffect();

            if (&out != &in)
                AudioBuffer::Copy(in, 0, out, 0, in.GetFrameCount());

            if (effect == nullptr)
                return;

            effect->GetFilter()->Process(in, out, in.GetFrameCount(), layer->GetSoundFormat().GetSampleRate());
        }
    };

    class EffectProcessor final : public SoundProcessor
    {
    public:
        EffectProcessor()
            : SoundProcessor("EffectProcessor")
        {}

        SoundProcessorInstance* CreateInstance() override
        {
            return ampoolnew(MemoryPoolKind::Amplimix, EffectProcessorInstance);
        }

        void DestroyInstance(SoundProcessorInstance* instance) override
        {
            ampooldelete(MemoryPoolKind::Amplimix, EffectProcessorInstance, (EffectProcessorInstance*)instance);
        }
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_MIXER_PIPELINE_EFFECT_PROCESSOR_H
