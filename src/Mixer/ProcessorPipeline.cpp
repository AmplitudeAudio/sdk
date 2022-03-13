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

#include <Mixer/ProcessorPipeline.h>

namespace SparkyStudios::Audio::Amplitude
{
    ProcessorPipeline::ProcessorPipeline()
        : _processors()
    {}

    ProcessorPipeline::~ProcessorPipeline()
    {
        _processors.clear();
    }

    void ProcessorPipeline::Append(SoundProcessor* processor)
    {
        _processors.push_back(processor);
    }

    void ProcessorPipeline::Insert(SoundProcessor* processor, AmSize index)
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
        AmInt16Buffer out,
        AmInt16Buffer in,
        AmUInt64 frames,
        AmUInt64 bufferSize,
        AmUInt16 channels,
        AmUInt32 sampleRate,
        SoundInstance* sound)
    {
        AmInt16Buffer cIn = in;

        for (auto&& p : _processors)
        {
            p->Process(out, cIn, frames, bufferSize, channels, sampleRate, sound);
            cIn = out;
        }
    }

    void ProcessorPipeline::ProcessInterleaved(
        AmInt16Buffer out,
        AmInt16Buffer in,
        AmUInt64 frames,
        AmUInt64 bufferSize,
        AmUInt16 channels,
        AmUInt32 sampleRate,
        SoundInstance* sound)
    {
        AmInt16Buffer cIn = in;

        for (auto&& p : _processors)
        {
            p->ProcessInterleaved(out, cIn, frames, bufferSize, channels, sampleRate, sound);
            cIn = out;
        }
    }

    void ProcessorPipeline::Cleanup(SoundInstance* sound)
    {
        for (auto&& p : _processors)
        {
            p->Cleanup(sound);
        }
    }
} // namespace SparkyStudios::Audio::Amplitude
