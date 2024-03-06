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
            SoundInstance* sound) override
        {
            const AmSize length = frames * channels;

#if defined(AM_SIMD_INTRINSICS)
            const AmSize end = AmAudioFrame::size * (length / AmAudioFrame::size);
            const auto lower = xsimd::batch(AM_AUDIO_SAMPLE_MIN), upper = xsimd::batch(AM_AUDIO_SAMPLE_MAX);

            for (AmSize i = 0; i < end; i += AmAudioFrame::size)
            {
                const auto bin = xsimd::load_aligned(&in[i]);

                xsimd::store_aligned(&out[i], xsimd::min(xsimd::max(bin, lower), upper));
            }

            for (AmSize i = end; i < length; i++)
            {
                out[i] = std::min(std::max(in[i], AM_AUDIO_SAMPLE_MIN), AM_AUDIO_SAMPLE_MAX);
            }
#else
            constexpr auto lower = AM_AUDIO_SAMPLE_MIN, upper = AM_AUDIO_SAMPLE_MAX;

            for (AmSize i = 0; i < length; i++)
            {
                out[i] = std::min(std::max(in[i], lower), upper);
            }
#endif // AM_SIMD_INTRINSICS
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
