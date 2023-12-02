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
#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>

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
        if (m_basePtr != nullptr)
            ampoolfree(MemoryPoolKind::Default, m_basePtr);

        if (size == 0)
            return AM_ERROR_NO_ERROR;

        m_basePtr = nullptr;
        m_data = nullptr;

        m_floats = size;

#ifndef AM_SIMD_INTRINSICS
        m_basePtr = static_cast<AmUInt8Buffer>(ammalloc(size * sizeof(AmReal32)));
#else
        m_basePtr = static_cast<AmUInt8Buffer>(ammalign(size * sizeof(AmReal32), AM_SIMD_ALIGNMENT));
#endif

        if (m_basePtr == nullptr)
            return AM_ERROR_OUT_OF_MEMORY;

        m_data = reinterpret_cast<AmReal32Buffer>(m_basePtr);

        return AM_ERROR_NO_ERROR;
    }

    void AmAlignedReal32Buffer::Clear() const
    {
        std::memset(m_basePtr, 0, sizeof(AmReal32) * m_floats);
    }

    void AmAlignedReal32Buffer::Release()
    {
        if (m_basePtr == nullptr)
            return;

        amfree(m_basePtr);
        m_basePtr = nullptr;

        m_floats = 0;
        m_data = nullptr;
    }

    void AmAlignedReal32Buffer::CopyFrom(const AmAlignedReal32Buffer& other) const
    {
        AMPLITUDE_ASSERT(m_floats == other.m_floats);

        if (this != &other)
        {
            std::memcpy(m_basePtr, other.m_basePtr, m_floats * sizeof(AmReal32));
        }
    }

    void AmAlignedReal32Buffer::Resize(AmUInt32 size)
    {
        if (m_basePtr == nullptr)
        {
            Init(size);
            return;
        }

        if (size == m_floats)
            return;

        m_floats = size;

#ifndef AM_SIMD_INTRINSICS
        m_basePtr = static_cast<AmUInt8Buffer>(amrealloc(m_basePtr, size * sizeof(AmReal32)));
#else
        m_basePtr = static_cast<AmUInt8Buffer>(amrealign(m_basePtr, size * sizeof(AmReal32), AM_SIMD_ALIGNMENT));
#endif

        m_data = reinterpret_cast<AmReal32Buffer>(m_basePtr);
    }

    void AmAlignedReal32Buffer::Swap(AmAlignedReal32Buffer& a, AmAlignedReal32Buffer& b)
    {
        std::swap(a.m_floats, b.m_floats);
        std::swap(a.m_data, b.m_data);
        std::swap(a.m_basePtr, b.m_basePtr);
    }

    AmAlignedReal32Buffer::~AmAlignedReal32Buffer()
    {
        Release();
    }

    AmTinyAlignedReal32Buffer::AmTinyAlignedReal32Buffer()
    {
        AmUInt8Buffer basePtr = &m_actualData[0];
        m_data = reinterpret_cast<AmReal32Buffer>(((size_t)basePtr + (AM_SIMD_ALIGNMENT - 1)) & ~(AM_SIMD_ALIGNMENT - 1));
    }

    void SoundFormat::SetAll(
        AmUInt32 sampleRate,
        AmUInt16 numChannels,
        AmUInt32 bitsPerSample,
        AmUInt64 framesCount,
        AmUInt32 frameSize,
        AM_SAMPLE_FORMAT sampleType)
    {
        _sampleRate = sampleRate;
        _numChannels = numChannels;
        _bitsPerSample = bitsPerSample;
        _framesCount = framesCount;
        _frameSize = frameSize;
        _sampleType = sampleType;
    }
} // namespace SparkyStudios::Audio::Amplitude