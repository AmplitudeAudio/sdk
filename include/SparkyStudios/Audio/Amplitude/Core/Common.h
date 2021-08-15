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

#ifndef SS_AMPLITUDE_AUDIO_COMMON_H
#define SS_AMPLITUDE_AUDIO_COMMON_H

#include <SparkyStudios/Audio/Amplitude/Core/Config.h>

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
// Common defines

// Define an invalid soloud object AmHandle
#define AM_INVALID_HANDLE 0

// Check for handle validity
#define AM_IS_VALID_HANDLE(handle) (handle != AM_INVALID_HANDLE)

// Check for channels value validity
#define AM_IS_VALID_CHANNELS_VALUE(channels) (channels == 1 || channels == 2 || channels == 4 || channels == 6 || channels == 8)

// Returns the minimum value between a and b
#define AM_MIN(a, b) ((a) < (b)) ? (a) : (b)

// Returns the maximum value between a and b
#define AM_MAX(a, b) ((a) > (b)) ? (a) : (b)

// Typedefs have to be made before the includes, as the
// includes depend on them.
namespace SparkyStudios::Audio::Amplitude
{
    class Engine;

    typedef void (*AmMutexCallFunction)(void* mutex);
    typedef void (*AmEngineCallFunction)(Engine* engine);
    typedef unsigned int AmResult;
    typedef unsigned int AmHandle;
    typedef double AmTime;

    typedef signed char AmInt8;
    typedef signed short AmInt16;
    typedef signed int AmInt32;
    typedef signed long long AmInt64;

    typedef unsigned char AmUInt8;
    typedef unsigned short AmUInt16;
    typedef unsigned int AmUInt32;
    typedef unsigned long long AmUInt64;

    typedef float* AmFloat32Buffer;

    typedef signed char* AmInt8Buffer;
    typedef signed short* AmInt16Buffer;
    typedef signed int* AmInt32Buffer;
    typedef signed long long* AmInt64Buffer;

    typedef unsigned char* AmUInt8Buffer;
    typedef unsigned short* AmUInt16Buffer;
    typedef unsigned int* AmUInt32Buffer;
    typedef unsigned long long* AmUInt64Buffer;

    typedef void* AmVoidPtr;

    typedef const char* AmString;
}; // namespace SparkyStudios::Audio::Amplitude

namespace SparkyStudios::Audio::Amplitude
{
    class Engine;

    /**
     * Class that handles aligned allocations to support vectorized operations.
     */
    class AmAlignedFloat32Buffer
    {
        friend class Engine;

    public:
        AmAlignedFloat32Buffer();
        ~AmAlignedFloat32Buffer();

        /**
         * Allocates and align buffer.
         * @param size The buffer size.
         * @return the result of the allocation.
         */
        AmResult Init(AmUInt32 size);

        /**
         * Clears all data.
         */
        void Clear() const;

        /**
         * Gets the size of the buffer.
         * @return AmUInt32
         */
        AmUInt32 GetSize() const
        {
            return m_floats;
        }

        /**
         * Gets the current aligned pointer.
         * @return AmFloat32Buffer
         */
        AmFloat32Buffer GetBuffer() const
        {
            return m_data;
        }

        /**
         * Gets the raw allocated pointer.
         * @return AmUInt8Buffer
         */
        AmUInt8Buffer GetPointer() const
        {
            return m_basePtr;
        }

    private:
        AmFloat32Buffer m_data; // aligned pointer
        AmUInt8Buffer m_basePtr; // raw allocated pointer (for delete)
        AmUInt32 m_floats; // size of buffer (w/out padding)
    };

    /**
     * Lightweight class that handles small aligned buffer to support vectorized operations.
     */
    class TinyAlignedFloatBuffer
    {
        friend class Engine;

    public:
        TinyAlignedFloatBuffer();

        AmFloat32Buffer m_data; // aligned pointer
        AmUInt8 m_actualData[sizeof(float) * 16 + 16]{};
    };
}; // namespace SparkyStudios::Audio::Amplitude

namespace SparkyStudios::Audio::Amplitude
{
    enum AM_ERROR : AmUInt8
    {
        AM_ERROR_NO_ERROR = 0, // No error
        AM_ERROR_INVALID_PARAMETER = 1, // Some parameter is invalid
        AM_ERROR_FILE_NOT_FOUND = 2, // File not found
        AM_ERROR_FILE_LOAD_FAILED = 3, // File found, but could not be loaded
        AM_ERROR_DLL_NOT_FOUND = 4, // DLL not found, or wrong DLL
        AM_ERROR_OUT_OF_MEMORY = 5, // Out of memory
        AM_ERROR_NOT_IMPLEMENTED = 6, // Feature not implemented
        AM_ERROR_UNKNOWN = 7 // Other error
    };

    // Enumerates the list of possible channel types handled by SoLoud
    enum AM_SPEAKER_CONFIG : AmUInt32
    {
        // Mono channel
        AM_SPEAKER_MONO,
        // Stereo channel
        AM_SPEAKER_STEREO,
        // Quad channel
        AM_SPEAKER_QUAD,
        // 5.1 Surround channel
        AM_SPEAKER_5_1,
        // 7.1 Surround channel
        AM_SPEAKER_7_1,
    };

    // Enumerates the list of possible sample formats handled by SoLoud
    enum AM_SAMPLE_FORMAT : AmUInt32
    {
        // floating point
        AM_SAMPLE_FORMAT_FLOAT,
        // integer
        AM_SAMPLE_FORMAT_INT,
        // An unknown format
        AM_SAMPLE_FORMAT_UNKNOWN,
    };

    enum AM_INTERLEAVE_TYPE : AmUInt32
    {
        AM_SAMPLE_INTERLEAVED,
        AM_SAMPLE_NON_INTERLEAVED,
    };

    enum AM_FADER_STATE : AmInt32
    {
        AM_FADER_STATE_STOPPED = -1,
        AM_FADER_STATE_DISABLED = 0,
        AM_FADER_STATE_ACTIVE = 1,
        AM_FADER_STATE_LFO = 2,
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_COMMON_H
