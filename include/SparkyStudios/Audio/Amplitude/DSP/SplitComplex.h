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

#ifndef _AM_DSP_SPLIT_COMPLEX_H
#define _AM_DSP_SPLIT_COMPLEX_H

#include <complex>
#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief Buffer for split-complex representation of FFT results.
     *
     * The split-complex representation stores the real and imaginary parts
     * of FFT results in two different memory buffers which is useful e.g. for
     * SIMD optimizations.
     *
     * @ingroup dsp
     */
    class AM_API_PUBLIC SplitComplex
    {
    public:
        /**
         * @brief Creates a new split-complex buffer with the given initial size.
         *
         * @param[in] initialSize The initial size of the split-complex buffer.
         */
        explicit SplitComplex(AmSize initialSize = 0);

        SplitComplex(const SplitComplex&) = delete;
        SplitComplex& operator=(const SplitComplex&) = delete;

        /**
         * @brief Destroy the split-complex buffer and release all allocated memory.
         */
        ~SplitComplex();

        /**
         * @brief Releases all allocated memory.
         */
        void Release();

        /**
         * @brief Resizes the split-complex buffer to the given size.
         *
         * @param[in] newSize The new size of the split-complex buffer.
         * @param[in] clear Whether to clear the buffer after resizing.
         */
        void Resize(AmSize newSize, bool clear = false);

        /**
         * @brief Clears the split-complex buffer.
         */
        void Clear() const;

        /**
         * @brief Copies the given split-complex buffer to this one.
         *
         * @param[in] other The split-complex buffer to copy.
         */
        void CopyFrom(const SplitComplex& other) const;

        /**
         * @brief Gets the current size of the split-complex buffer.
         *
         * @return The size of the split-complex buffer.
         */
        [[nodiscard]] AM_INLINE AmSize GetSize() const
        {
            return _size;
        }

        /**
         * @brief Gets the real part of the split-complex buffer.
         *
         * @return The real part of the split-complex buffer.
         */
        AmAudioSample* re();

        /**
         * @brief Gets the real part of the split-complex buffer.
         *
         * @return The real part of the split-complex buffer.
         */
        [[nodiscard]] const AmAudioSample* re() const;

        /**
         * @brief Gets the imaginary part of the split-complex buffer.
         *
         * @return The imaginary part of the split-complex buffer.
         */
        AmAudioSample* im();

        /**
         * @brief Gets the imaginary part of the split-complex buffer.
         *
         * @return The imaginary part of the split-complex buffer.
         */
        [[nodiscard]] const AmAudioSample* im() const;

        /**
         * @brief Gets the complex value at the given index.
         *
         * @param[in] index The index of the complex value.
         *
         * @return The complex value at the given index.
         */
        [[nodiscard]] std::complex<AmAudioSample> operator[](AmSize index) const;

    private:
        AmSize _size;
        AmAlignedReal32Buffer _re;
        AmAlignedReal32Buffer _im;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_DSP_SPLIT_COMPLEX_H