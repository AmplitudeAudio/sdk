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

#ifndef _AM_MATH_FFT_H
#define _AM_MATH_FFT_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

#include <SparkyStudios/Audio/Amplitude/Math/SplitComplex.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief The Fast Fourier Transform (FFT) class.
     *
     * This utility class is used to perform Fast Fourier Transform (FFT) operations
     * on audio data with real-to-complex/complex-to-real routines. The algorithm is
     * highly optimized for speed, and the class provides and high-level API for the
     * user.
     *
     * The output of the operation is ready-to-use, that means all the post processing
     * operations (scale, normalization, etc.) have been applied.
     */
    class AM_API_PUBLIC FFT
    {
    public:
        /**
         * @brief Gets the FFT output buffer size.
         *
         * @param inputSize The size of the input buffer.
         *
         * @return The size of the FFT output buffer for the given input size.
         */
        static AmUInt64 GetOutputSize(AmUInt64 inputSize);

        /**
         * @brief The default constructor.
         */
        FFT();

        FFT(const FFT&) = delete;
        FFT& operator=(const FFT&) = delete;

        /**
         * @brief Destructor.
         */
        ~FFT();

        /**
         * @brief Initializes the FFT instance.
         *
         * @param size The size of the input (as a power of 2).
         */
        void Initialize(AmSize size) const;

        /**
         * @brief Performs the forward FFT operation.
         *
         * @param input The input audio data. This buffer needs to be of the same size as the one provided to the Initialize() method.
         * @param splitComplex The complex buffer output separated into real and imaginary parts. The buffer will be resized if necessary.
         */
        void Forward(AmConstAudioSampleBuffer input, SplitComplex& splitComplex) const;

        /**
         * @brief Performs the inverse FFT operation.
         *
         * @param output The output audio data. This buffer needs to be of the same size as the one provided to the Initialize() method.
         * @param splitComplex The complex buffer output separated into real and imaginary parts. The buffer will be resized if necessary.
         */
        void Backward(AmAudioSampleBuffer output, SplitComplex& splitComplex) const;

    private:
        void* _implementation;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_MATH_FFT_H
