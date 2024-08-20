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

#ifndef _AM_MIXER_PIPELINE_H
#define _AM_MIXER_PIPELINE_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>
#include <SparkyStudios/Audio/Amplitude/Mixer/Amplimix.h>
#include <SparkyStudios/Audio/Amplitude/Mixer/SoundProcessor.h>

namespace SparkyStudios::Audio::Amplitude
{
    class Pipeline
    {
    public:
        Pipeline();
        ~Pipeline();

        void Append(SoundProcessorInstance* processor);

        void Insert(SoundProcessorInstance* processor, AmSize index);

        void Process(const AmplimixLayer* layer, const AudioBuffer& in, AudioBuffer& out);

        void Cleanup(const AmplimixLayer* layer);

    private:
        std::vector<SoundProcessorInstance*> _processors;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_MIXER_PIPELINE_H
