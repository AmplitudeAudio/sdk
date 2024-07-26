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
#include <SparkyStudios/Audio/Amplitude/Mixer/Amplimix.h>

#include <Mixer/ProcessorPipeline.h>

namespace SparkyStudios::Audio::Amplitude
{
    ProcessorPipeline::ProcessorPipeline()
        : _processors()
    {}

    ProcessorPipeline::~ProcessorPipeline()
    {
        for (const auto& processor : _processors)
            ampooldelete(MemoryPoolKind::Amplimix, SoundProcessorInstance, processor);

        _processors.clear();
    }

    void ProcessorPipeline::Append(SoundProcessorInstance* processor)
    {
        _processors.push_back(processor);
    }

    void ProcessorPipeline::Insert(SoundProcessorInstance* processor, AmSize index)
    {
        if (index >= _processors.size())
        {
            _processors.push_back(processor);
        }
        else
        {
            _processors.insert(_processors.begin() + index, processor);
        }
    }

    void ProcessorPipeline::Process(
        AmAudioSampleBuffer out,
        AmConstAudioSampleBuffer in,
        AmUInt64 frames,
        AmSize bufferSize,
        AmUInt16 channels,
        AmUInt32 sampleRate,
        const AmplimixLayer* layer)
    {
        AmConstAudioSampleBuffer cIn = in;

        for (auto&& p : _processors)
        {
            p->Process(out, cIn, frames, bufferSize, channels, sampleRate, layer);
            cIn = out;
        }
    }

    void ProcessorPipeline::Cleanup(const AmplimixLayer* layer)
    {
        for (auto&& p : _processors)
            p->Cleanup(layer);
    }

    AmSize ProcessorPipeline::GetOutputBufferSize(AmUInt64 frames, AmSize bufferSize, AmUInt16 channels, AmUInt32 sampleRate)
    {
        AmSize outputSize = 0;
        for (auto&& p : _processors)
            outputSize = AM_MAX(p->GetOutputBufferSize(frames, bufferSize, channels, sampleRate), outputSize);

        return outputSize;
    }
} // namespace SparkyStudios::Audio::Amplitude
