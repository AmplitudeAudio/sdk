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

#ifndef _AM_CORE_COMMON_H
#define _AM_CORE_COMMON_H

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
// Standard Library

#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <map>
#include <memory>
#include <numeric>
#include <ranges>
#include <set>
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
#define AM_INVALID_HANDLE nullptr

// Check for handle validity
#define AM_IS_VALID_HANDLE(handle) ((handle) != AM_INVALID_HANDLE)

// Clamps a value between a and b
#define AM_CLAMP(v, a, b) (((v) < (a)) ? (a) : ((v) > (b)) ? (b) : (v))

// Checks if a value is between a and b
#define AM_BETWEEN(v, a, b) ((v) >= AM_MIN(a, b) && (v) <= AM_MAX(a, b))

/**
 * @brief Declare a callback function type
 *
 * @param _type_ Return type of the function
 * @param _name_ Name of the function
 *
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
 * @brief Helps to avoid compiler warnings about unused values.
 *
 * @param x The statement where the return value is not used.
 */
#define AM_UNUSED(x) ((void)(x))

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief Class that handles aligned allocations to support vectorized operations.
     */
    class AM_API_PUBLIC AmAlignedReal32Buffer
    {
    public:
        AmAlignedReal32Buffer();
        ~AmAlignedReal32Buffer();

        /**
         * @brief Allocates and align buffer.
         *
         * @param size The buffer size.
         * @param clear Whether to clear the buffer.
         *
         * @return the result of the allocation.
         */
        AmResult Init(AmUInt32 size, bool clear = true);

        /**
         * @brief Clears all data.
         */
        void Clear() const;

        /**
         * @brief Releases the allocated buffer.
         */
        void Release();

        /**
         * @brief Gets the size of the buffer.
         *
         * @return AmUInt32
         */
        [[nodiscard]] AM_INLINE AmUInt32 GetSize() const
        {
            return m_floats;
        }

        /**
         * @brief Gets the current aligned pointer.
         *
         * @return AmFloat32Buffer
         */
        [[nodiscard]] AM_INLINE AmReal32Buffer GetBuffer() const
        {
            return m_data;
        }

        /**
         * @brief Gets the raw allocated pointer.
         *
         * @return AmUInt8Buffer
         */
        [[nodiscard]] AM_INLINE AmUInt8Buffer GetPointer() const
        {
            return m_basePtr;
        }

        /**
         * @brief Copies data from another buffer.
         * @param other The other buffer to copy data from.
         */
        void CopyFrom(const AmAlignedReal32Buffer& other) const;

        /**
         * @brief Resizes the buffer to the specified size.
         *
         * @param size The new size of the buffer.
         * @param clear Whether to clear the buffer after resize. If @c true, the buffer will be cleared
         * even if the new size equals the old size.
         */
        void Resize(AmUInt32 size, bool clear = true);

        /**
         * @brief Swaps two buffers.
         *
         * @param a The first buffer.
         * @param b The second buffer.
         */
        static void Swap(AmAlignedReal32Buffer& a, AmAlignedReal32Buffer& b);

        AmReal32& operator[](AmSize index)
        {
            AMPLITUDE_ASSERT(m_data != nullptr && index < m_floats);
            return m_data[index];
        }

        const AmReal32& operator[](AmSize index) const
        {
            AMPLITUDE_ASSERT(m_data != nullptr && index < m_floats);
            return m_data[index];
        }

    private:
        AmReal32Buffer m_data; // aligned pointer
        AmUInt8Buffer m_basePtr; // raw allocated pointer (for delete)
        AmUInt32 m_floats; // size of buffer (w/out padding)
    };

    /**
     * @brief Lightweight class that handles small aligned buffer to support vectorized operations.
     */
    class AM_API_PUBLIC AmTinyAlignedReal32Buffer
    {
    public:
        /**
         * @brief Construct a new TinyAlignedReal32Buffer object by allocating an aligned
         * buffer with AM_SIMD_ALIGNMENT float values.
         */
        AmTinyAlignedReal32Buffer();

        /**
         * @brief Gets the raw allocated pointer.
         *
         * @return AmReal32Buffer
         */
        [[nodiscard]] AM_INLINE AmReal32Buffer GetBuffer() const
        {
            return m_data;
        }

    private:
        AmReal32Buffer m_data; // aligned pointer
        AmUInt8 m_actualData[sizeof(AmReal32) * AM_SIMD_ALIGNMENT + AM_SIMD_ALIGNMENT]{};
    };

    /**
     * @brief Enumerates the list of possible errors encountered by the library.
     */
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

    /**
     * @brief Enumerates the list of possible sample formats handled by Amplitude.
     */
    enum AM_SAMPLE_FORMAT : AmUInt8
    {
        // floating point (float32)
        AM_SAMPLE_FORMAT_FLOAT,
        // integer (int16)
        AM_SAMPLE_FORMAT_INT,
        // An unknown format
        AM_SAMPLE_FORMAT_UNKNOWN,
    };

    /**
     * @brief Enumerates the list of states in a fader.
     */
    enum AM_FADER_STATE : AmInt8
    {
        AM_FADER_STATE_STOPPED = -1,
        AM_FADER_STATE_DISABLED = 0,
        AM_FADER_STATE_ACTIVE = 1,
    };

    /**
     * @brief Enumerates the list of available spatialization modes.
     */
    enum eSpatialization : AmUInt8
    {
        /**
         * @brief Disables spatialization.
         */
        eSpatialization_None,

        /**
         * @brief Enables 2D (left-right) spatialization based on sound position.
         *
         * @note This mode is available for every panning mode.
         */
        eSpatialization_Position,

        /**
         * @brief Enables 2D (left-right) spatialization based on sound position
         * and orientation.
         *
         * @note The sound instance using this spatialization mode needs to be attached to an @c Entity.
         * @note This mode is available for every panning mode.
         */
        eSpatialization_PositionOrientation,

        /**
         * @brief Enables 3D spatialization using Head Related Transfer Functions.
         *
         * @note This mode is only available for binaural panning modes.
         */
        eSpatialization_HRTF
    };

    /**
     * @brief Enumerates the list of available panning modes.
     */
    enum ePanningMode : AmUInt8
    {
        /**
         * @brief 2D stereo panning. This panning mode won't provide HRTF-related features.
         *
         * @note The Ambisonic decoder will use a virtual array of 2 loudspeakers
         * evenly arranged in front of the listener's head.
         */
        ePanningMode_Stereo = 0,

        /**
         * @brief 3D binaural panning using first-order HRTF.
         *
         * @note The Ambisonic decoder will use a virtual array of 8 loudspeakers
         * arranged in a cube configuration around the listener's head.
         */
        ePanningMode_BinauralLowQuality = 1,

        /**
         * @brief 3D binaural panning using second-order HRTF.
         *
         * @note The Ambisonic decoder will use a virtual array of 12 loudspeakers
         * arranged in a dodecahedral configuration (using faces of the dodecahedron).
         */
        ePanningMode_BinauralMediumQuality = 2,

        /**
         * @brief 3D binaural panning using third-order HRTF.
         *
         * @note The Ambisonic decoder will use a virtual array of 26 loudspeakers
         * arranged in a Lebedev grid. See: https://people.sc.fsu.edu/~jburkardt/m_src/sphere_lebedev_rule/sphere_lebedev_rule.html
         */
        ePanningMode_BinauralHighQuality = 3,
    };

    /**
     * @brief Defines how the HRIR sphere is sampled when doing Ambisonics binauralization.
     *
     * @ingroup core
     */
    enum eHRIRSphereSamplingMode : AmUInt8
    {
        /**
         * @brief Provides the most accurate binauralization, as the HRIR data are smoothly transitioned between sphere points.
         *
         * See more info about bilinear sampling [here](http://www02.smt.ufrj.br/~diniz/conf/confi117.pdf).
         */
        eHRIRSphereSamplingMode_Bilinear = 0,

        /**
         * @brief Provides a more efficient binauralization, as the HRIR data are interpolated using only the nearest neighbors.
         */
        eHRIRSphereSamplingMode_NearestNeighbor = 1,
    };

    /**
     * @brief Describe the format of an audio sample.
     *
     * This data structure is mainly filled by a Codec
     * during the initialization time.
     */
    struct AM_API_PUBLIC SoundFormat
    {
    public:
        void SetAll(
            AmUInt32 sampleRate,
            AmUInt16 numChannels,
            AmUInt32 bitsPerSample,
            AmUInt64 framesCount,
            AmUInt32 frameSize,
            AM_SAMPLE_FORMAT sampleType);

        /**
         * @brief Get the sample rate.
         *
         * @return AmUInt32
         */
        [[nodiscard]] AM_INLINE AmUInt32 GetSampleRate() const
        {
            return _sampleRate;
        }

        /**
         * @brief Get the number of channels.
         *
         * @return AmUInt16
         */
        [[nodiscard]] AM_INLINE AmUInt16 GetNumChannels() const
        {
            return _numChannels;
        }

        /**
         * @brief Get the bits per sample.
         *
         * @return AmUInt32
         */
        [[nodiscard]] AM_INLINE AmUInt32 GetBitsPerSample() const
        {
            return _bitsPerSample;
        }

        /**
         * @brief Get the number of frames.
         *
         * @return AmUInt64
         */
        [[nodiscard]] AM_INLINE AmUInt64 GetFramesCount() const
        {
            return _framesCount;
        }

        /**
         * @brief Get the frame size.
         *
         * @return AmUInt32
         */
        [[nodiscard]] AM_INLINE AmUInt32 GetFrameSize() const
        {
            return _frameSize;
        }

        /**
         * @brief Get the sample type.
         *
         * @return AM_SAMPLE_FORMAT
         */
        [[nodiscard]] AM_INLINE AM_SAMPLE_FORMAT GetSampleType() const
        {
            return _sampleType;
        }

    private:
        AmUInt32 _sampleRate = 0;
        AmUInt16 _numChannels = 0;
        AmUInt32 _bitsPerSample = 0;
        AmUInt64 _framesCount = 0;
        AmUInt32 _frameSize = 0;
        AM_SAMPLE_FORMAT _sampleType = AM_SAMPLE_FORMAT_FLOAT;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_CORE_COMMON_H
