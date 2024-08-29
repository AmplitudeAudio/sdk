// Copyright (c) 2024-present Sparky Studios. All rights reserved.
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

#ifndef _AM_IMPLEMENTATION_DSP_NEAR_FIELD_PROCESSOR_H
#define _AM_IMPLEMENTATION_DSP_NEAR_FIELD_PROCESSOR_H

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <DSP/Delay.h>
#include <DSP/Filters/BiquadResonantFilter.h>

namespace SparkyStudios::Audio::Amplitude
{
    class NearFieldProcessor
    {
    public:
        NearFieldProcessor(AmUInt32 sampleRate, AmUInt32 framesCount);

        void Process(const AudioBufferChannel& in, AudioBufferChannel& out, bool isHrtfEnabled);

    private:
        // Sample rate.
        AmUInt32 _sampleRate = 0;

        // Number of frames per buffer.
        AmSize _framesCount = 0;

        // Delay compensation computed as average group delay of the HRTF filter
        // minus average group delay of the shelf-filter. Should be disabled when
        // using with stereo-panned sound sources.
        AmSize _delayCompensation = 0;

        // Biquad filters that apply frequency splitting of the input mono signal.
        FilterInstance* _lowPassFilter = nullptr;
        FilterInstance* _highPassFilter = nullptr;

        // Buffer for the low-passed signal. We do not modify the high-passed signal
        // so we can write it directly to the output channel.
        AudioBuffer _lowPassBuffer;

        // Delay used to delay the incoming input mono buffer.
        Delay _delay;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_MIXER_PIPELINE_NEAR_FIELD_PROCESSOR_H
