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

#ifndef SS_AMPLITUDE_AUDIO_MIXERPIPELINE_H
#define SS_AMPLITUDE_AUDIO_MIXERPIPELINE_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>
#include <SparkyStudios/Audio/Amplitude/Mixer/SoundProcessor.h>

namespace SparkyStudios::Audio::Amplitude
{
    class ProcessorPipeline : SoundProcessorInstance
    {
    public:
        ProcessorPipeline();
        ~ProcessorPipeline() override;

        void Append(SoundProcessorInstance* processor);

        void Insert(SoundProcessorInstance* processor, AmSize index);

        void Process(
            AmAudioSampleBuffer out,
            AmConstAudioSampleBuffer in,
            AmUInt64 frames,
            AmSize bufferSize,
            AmUInt16 channels,
            AmUInt32 sampleRate,
            SoundInstance* sound) override;

        void ProcessInterleaved(
            AmAudioSampleBuffer out,
            AmConstAudioSampleBuffer in,
            AmUInt64 frames,
            AmSize bufferSize,
            AmUInt16 channels,
            AmUInt32 sampleRate,
            SoundInstance* sound) override;

        void Cleanup(SoundInstance* sound) override;

        AmSize GetOutputBufferSize(AmUInt64 frames, AmSize bufferSize, AmUInt16 channels, AmUInt32 sampleRate) override;

    private:
        std::vector<SoundProcessorInstance*> _processors;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_MIXERPIPELINE_H
