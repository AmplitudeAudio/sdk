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

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

namespace SparkyStudios::Audio::Amplitude
{
    AmAlignedFloat32Buffer::AmAlignedFloat32Buffer()
    {
        m_basePtr = nullptr;
        m_data = nullptr;
        m_floats = 0;
    }

    AmResult AmAlignedFloat32Buffer::Init(AmUInt32 size)
    {
        delete[] m_basePtr;
        m_basePtr = nullptr;
        m_data = nullptr;
        m_floats = size;
#ifndef AMPLITUDE_SSE_INTRINSICS
        m_basePtr = new unsigned char[size * sizeof(float)];
        if (m_basePtr == nullptr)
            return AM_ERROR_OUT_OF_MEMORY;
        m_data = (float*)m_basePtr;
#else
        m_basePtr = new unsigned char[size * sizeof(float) + 16];
        if (m_basePtr == nullptr)
            return AM_ERROR_OUT_OF_MEMORY;
        m_data = (AmFloat32Buffer)(((size_t)m_basePtr + 15) & ~15);
#endif
        return AM_ERROR_NO_ERROR;
    }

    void AmAlignedFloat32Buffer::Clear() const
    {
        memset(m_data, 0, sizeof(float) * m_floats);
    }

    AmAlignedFloat32Buffer::~AmAlignedFloat32Buffer()
    {
        delete[] m_basePtr;
    }

    TinyAlignedFloatBuffer::TinyAlignedFloatBuffer()
    {
        AmUInt8Buffer basePtr = &m_actualData[0];
        m_data = (AmFloat32Buffer)(((size_t)basePtr + 15) & ~15);
    }

} // namespace SparkyStudios::Audio::Amplitude