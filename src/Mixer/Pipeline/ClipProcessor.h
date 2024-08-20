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

#ifndef _AM_IMPLEMENTATION_MIXER_PIPELINE_CLIP_PROCESSOR_H
#define _AM_IMPLEMENTATION_MIXER_PIPELINE_CLIP_PROCESSOR_H

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Utils/Utils.h>

namespace SparkyStudios::Audio::Amplitude
{
    class ClipProcessorInstance : public SoundProcessorInstance
    {
    public:
        void Process(const AmplimixLayer* layer, const AudioBuffer& in, AudioBuffer& out) override
        {
            const AmSize length = in.GetFrameCount();
            const AmSize channels = in.GetChannelCount();

            for (AmSize c = 0; c < channels; c++)
            {
                const auto& inChannel = in[c];
                auto& outChannel = out[c];

                for (AmSize i = 0; i < length; i++)
                    outChannel[i] = inChannel[i] <= -1.65f ? -0.9862875f
                        : inChannel[i] >= 1.65f            ? 0.9862875f
                                                           : 0.87f * inChannel[i] - 0.1f * inChannel[i] * inChannel[i] * inChannel[i];
            }
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

#endif // _AM_IMPLEMENTATION_MIXER_PIPELINE_CLIP_PROCESSOR_H
