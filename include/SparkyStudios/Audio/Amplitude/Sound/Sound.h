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

#ifndef SS_AMPLITUDE_AUDIO_SOUND_H
#define SS_AMPLITUDE_AUDIO_SOUND_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief Describe the format of an audio sample.
     *
     * This data structure is mainly filled by a Codec
     * during the initialization time.
     */
    struct SoundFormat
    {
    public:
        void SetAll(
            AmUInt32 sampleRate,
            AmUInt16 numChannels,
            AmUInt32 bitsPerSample,
            AmUInt32 frameSize,
            AM_SAMPLE_FORMAT sampleType,
            AM_INTERLEAVE_TYPE interleaveType);

        [[nodiscard]] AmUInt32 GetSampleRate() const
        {
            return _sampleRate;
        }

        [[nodiscard]] AmUInt16 GetNumChannels() const
        {
            return _numChannels;
        }

        [[nodiscard]] AmUInt32 GetBitsPerSample() const
        {
            return _bitsPerSample;
        }

        [[nodiscard]] AmUInt32 GetFrameSize() const
        {
            return _frameSize;
        }

        [[nodiscard]] AM_SAMPLE_FORMAT GetSampleType() const
        {
            return _sampleType;
        }

        [[nodiscard]] AM_INTERLEAVE_TYPE GetInterleaveType() const
        {
            return _interleaveType;
        }

    private:
        AmUInt32 _sampleRate;
        AmUInt16 _numChannels;
        AmUInt32 _bitsPerSample;
        AmUInt32 _frameSize;
        AM_SAMPLE_FORMAT _sampleType;
        AM_INTERLEAVE_TYPE _interleaveType;
    };

} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_SOUND_H
