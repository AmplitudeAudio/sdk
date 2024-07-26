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

#ifndef SS_AMPLITUDE_AUDIO_CLIP_PROCESSOR_H
#define SS_AMPLITUDE_AUDIO_CLIP_PROCESSOR_H

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Utils/Utils.h>

namespace SparkyStudios::Audio::Amplitude
{
    class ClipProcessorInstance : public SoundProcessorInstance
    {
    public:
        void Process(
            AmAudioSampleBuffer out,
            AmConstAudioSampleBuffer in,
            AmUInt64 frames,
            AmSize bufferSize,
            AmUInt16 channels,
            AmUInt32 sampleRate,
            const AmplimixLayer* layer) override
        {
            const AmSize length = frames * channels;

            for (AmSize i = 0; i < length; i++)
                out[i] = in[i] <= -1.65f ? -0.9862875f : in[i] >= 1.65f ? 0.9862875f : 0.87f * in[i] - 0.1f * in[i] * in[i] * in[i];
        }
    };

    class ClipProcessor final : public SoundProcessor
    {
    public:
        ClipProcessor()
            : SoundProcessor("ClipProcessor")
        {}

        SoundProcessorInstance* CreateInstance() override
        {
            return ampoolnew(MemoryPoolKind::Amplimix, ClipProcessorInstance);
        }

        void DestroyInstance(SoundProcessorInstance* instance) override
        {
            ampooldelete(MemoryPoolKind::Amplimix, ClipProcessorInstance, (ClipProcessorInstance*)instance);
        }
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_CLIP_PROCESSOR_H
