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

#include <cstring>

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

namespace SparkyStudios::Audio::Amplitude
{
    AmAlignedReal32Buffer::AmAlignedReal32Buffer()
    {
        m_basePtr = nullptr;
        m_data = nullptr;
        m_floats = 0;
    }

    AmResult AmAlignedReal32Buffer::Init(AmUInt32 size)
    {
        delete[] m_basePtr;
        m_basePtr = nullptr;
        m_data = nullptr;
        m_floats = size;
#ifndef AM_SIMD_INTRINSICS
        m_basePtr = new AmUInt8[size * sizeof(float)];
        if (m_basePtr == nullptr)
            return AM_ERROR_OUT_OF_MEMORY;
        m_data = (AmReal32Buffer)m_basePtr;
#else
        m_basePtr = new AmUInt8[size * sizeof(float) + AM_SIMD_ALIGNMENT];
        if (m_basePtr == nullptr)
            return AM_ERROR_OUT_OF_MEMORY;
        m_data = (AmReal32Buffer)(((size_t)m_basePtr + (AM_SIMD_ALIGNMENT - 1)) & ~(AM_SIMD_ALIGNMENT - 1));
#endif
        return AM_ERROR_NO_ERROR;
    }

    void AmAlignedReal32Buffer::Clear() const
    {
        memset(m_data, 0, sizeof(float) * m_floats);
    }

    AmAlignedReal32Buffer::~AmAlignedReal32Buffer()
    {
        delete[] m_basePtr;
    }

    TinyAlignedReal32Buffer::TinyAlignedReal32Buffer()
    {
        AmUInt8Buffer basePtr = &m_actualData[0];
        m_data = (AmReal32Buffer)(((size_t)basePtr + (AM_SIMD_ALIGNMENT - 1)) & ~(AM_SIMD_ALIGNMENT - 1));
    }

    void SoundFormat::SetAll(
        AmUInt32 sampleRate,
        AmUInt16 numChannels,
        AmUInt32 bitsPerSample,
        AmUInt64 framesCount,
        AmUInt32 frameSize,
        AM_SAMPLE_FORMAT sampleType,
        AM_INTERLEAVE_TYPE interleaveType)
    {
        _sampleRate = sampleRate;
        _numChannels = numChannels;
        _bitsPerSample = bitsPerSample;
        _framesCount = framesCount;
        _frameSize = frameSize;
        _sampleType = sampleType;
        _interleaveType = interleaveType;
    }
} // namespace SparkyStudios::Audio::Amplitude