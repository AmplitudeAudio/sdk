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

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
// Standard Library

#include <atomic>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
// Amplitude

#include <SparkyStudios/Audio/Amplitude/Core/Common/Config.h>
#include <SparkyStudios/Audio/Amplitude/Core/Common/Types.h>

#include <SparkyStudios/Audio/Amplitude/Core/Common/Constants.h>

#include <SparkyStudios/Audio/Amplitude/Math/HandmadeMath.h>

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
// Common defines

// Define an invalid object handle
#define AM_INVALID_HANDLE 0

// Check for handle validity
#define AM_IS_VALID_HANDLE(handle) ((handle) != AM_INVALID_HANDLE)

// Check for channels value validity
#define AM_IS_VALID_CHANNELS_VALUE(channels) ((channels) == 1 || (channels) == 2 || (channels) == 4 || (channels) == 6 || (channels) == 8)

// Clamps a value between a and b
#define AM_CLAMP(v, a, b) (((v) < (a)) ? (a) : ((v) > (b)) ? (b) : (v))

// Checks if a value is between a and b
#define AM_BETWEEN(v, a, b) ((v) >= AM_MIN(a, b) && (v) <= AM_MAX(a, b))

/**
 * @brief Declare a callback function type
 * @param _type_ Return type of the function
 * @param _name_ Name of the function
 * @remarks This must be followed by the parentheses containing the function arguments declaration
 */
#define AM_CALLBACK(_type_, _name_) typedef _type_(AM_CALL_POLICY* _name_)

/**
 * @brief The minimum value for an audio sample.
 */
#define AM_AUDIO_SAMPLE_MIN (-1.0f)

/**
 * @brief The maximum value for an audio sample.
 */
#define AM_AUDIO_SAMPLE_MAX (+1.0f)

/**
 * @brief Clamps an audio sample value between AM_AUDIO_SAMPLE_MIN and AM_AUDIO_SAMPLE_MAX.
 */
#define AM_CLAMP_AUDIO_SAMPLE(v) ((v) <= -1.65f) ? -0.9862875f : ((v) >= 1.65f) ? 0.9862875f : (0.87f * (v)-0.1f * (v) * (v) * (v))

// Typedefs have to be made before the includes, as the
// includes depend on them.

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief Class that handles aligned allocations to support vectorized operations.
     */
    class AmAlignedReal32Buffer
    {
    public:
        AmAlignedReal32Buffer();
        ~AmAlignedReal32Buffer();

        /**
         * @brief Allocates and align buffer.
         *
         * @param size The buffer size.
         * @return the result of the allocation.
         */
        AmResult Init(AmUInt32 size);

        /**
         * @brief Clears all data.
         */
        void Clear() const;

        /**
         * @brief Gets the size of the buffer.
         *
         * @return AmUInt32
         */
        [[nodiscard]] AmUInt32 GetSize() const
        {
            return m_floats;
        }

        /**
         * @brief Gets the current aligned pointer.
         *
         * @return AmFloat32Buffer
         */
        [[nodiscard]] AmReal32Buffer GetBuffer() const
        {
            return m_data;
        }

        /**
         * @brief Gets the raw allocated pointer.
         *
         * @return AmUInt8Buffer
         */
        [[nodiscard]] AmUInt8Buffer GetPointer() const
        {
            return m_basePtr;
        }

    private:
        AmReal32Buffer m_data; // aligned pointer
        AmUInt8Buffer m_basePtr; // raw allocated pointer (for delete)
        AmUInt32 m_floats; // size of buffer (w/out padding)
    };

    /**
     * @brief Lightweight class that handles small aligned buffer to support vectorized operations.
     */
    class TinyAlignedReal32Buffer
    {
    public:
        TinyAlignedReal32Buffer();

    private:
        AmReal32Buffer m_data; // aligned pointer
        AmUInt8 m_actualData[sizeof(float) * AM_SIMD_ALIGNMENT + AM_SIMD_ALIGNMENT]{};
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
    enum AM_SPEAKER_CONFIG : AmUInt8
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

    // Enumerates the list of possible sample formats handled by Amplitude
    enum AM_SAMPLE_FORMAT : AmUInt8
    {
        // floating point
        AM_SAMPLE_FORMAT_FLOAT,
        // integer
        AM_SAMPLE_FORMAT_INT,
        // An unknown format
        AM_SAMPLE_FORMAT_UNKNOWN,
    };

    enum AM_INTERLEAVE_TYPE : AmUInt8
    {
        AM_SAMPLE_INTERLEAVED,
        AM_SAMPLE_NON_INTERLEAVED,
    };

    enum AM_FADER_STATE : AmInt8
    {
        AM_FADER_STATE_STOPPED = -1,
        AM_FADER_STATE_DISABLED = 0,
        AM_FADER_STATE_ACTIVE = 1,
    };
} // namespace SparkyStudios::Audio::Amplitude

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
            AmUInt64 framesCount,
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

        [[nodiscard]] AmUInt64 GetFramesCount() const
        {
            return _framesCount;
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
        AmUInt64 _framesCount;
        AmUInt32 _frameSize;
        AM_SAMPLE_FORMAT _sampleType;
        AM_INTERLEAVE_TYPE _interleaveType;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_COMMON_H
