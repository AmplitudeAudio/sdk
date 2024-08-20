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
#include <SparkyStudios/Audio/Amplitude/Mixer/Pipeline.h>

namespace SparkyStudios::Audio::Amplitude
{
    Pipeline::Pipeline()
        : _processors()
    {}

    Pipeline::~Pipeline()
    {
        for (const auto& processor : _processors)
            ampooldelete(MemoryPoolKind::Amplimix, SoundProcessorInstance, processor);

        _processors.clear();
    }

    void Pipeline::Append(SoundProcessorInstance* processor)
    {
        _processors.push_back(processor);
    }

    void Pipeline::Insert(SoundProcessorInstance* processor, AmSize index)
    {
        if (index >= _processors.size())
            _processors.push_back(processor);
        else
            _processors.insert(_processors.begin() + index, processor);
    }

    void Pipeline::Process(const AmplimixLayer* layer, const AudioBuffer& in, AudioBuffer& out)
    {
        AMPLITUDE_ASSERT(in.GetChannelCount() == out.GetChannelCount());
        AMPLITUDE_ASSERT(in.GetFrameCount() == out.GetFrameCount());

        const AudioBuffer* cIn = &in;

        for (auto&& p : _processors)
        {
            p->Process(layer, *cIn, out);
            cIn = &out;
        }
    }

    void Pipeline::Cleanup(const AmplimixLayer* layer)
    {
        for (auto&& p : _processors)
            p->Cleanup(layer);
    }
} // namespace SparkyStudios::Audio::Amplitude
