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
//
// Based on the code from https://github.com/HiFi-LoFi/FFTConvolver
// Copyright (c) 2017 HiFi-LoFi, MIT License

#pragma once

#ifndef _AM_DSP_CONVOLVER_H
#define _AM_DSP_CONVOLVER_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

#include <SparkyStudios/Audio/Amplitude/DSP/FFT.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief Implementation of a partitioned FFT convolution algorithm with uniform block size.
     *
     * Some notes on how to use it:
     *
     * - After initialization with an impulse response, subsequent data portions of
     *   arbitrary length can be convolved. The convolver internally can handle
     *   this by using appropriate buffering.
     *
     * - The convolver works without "latency" (except for the required
     *   processing time, of course), i.e. the output always is the convolved
     *   input for each processing call.
     *
     * - The convolver is suitable for real-time processing which means that no
     *   "unpredictable" operations like allocations, locking, API calls, etc. are
     *   performed during processing (all necessary allocations and preparations take
     *   place during initialization).
     *
     * @ingroup dsp
     */
    class Convolver
    {
    public:
        /**
         * @brief Default constructor.
         *
         * Creates an uninitialized convolver.
         */
        Convolver();

        /**
         * @brief Destructor.
         *
         * Destroys the convolver and frees all allocated resources.
         */
        virtual ~Convolver();

        // Prevent uncontrolled usage
        Convolver(const Convolver&) = delete;
        Convolver& operator=(const Convolver&) = delete;

        /**
         * @brief Initializes the convolver.
         *
         * @param[in] blockSize Block size internally used by the convolver (partition size)
         * @param[in] ir The impulse response
         * @param[in] irLen Length of the impulse response
         *
         * @return `true` when the convolver is successfully initialized, `false` otherwise.
         */
        bool Init(AmSize blockSize, const AmAudioSample* ir, AmSize irLen);

        /**
         * @brief Convolves the the given input samples and immediately outputs the result.
         *
         * @param[in] input The input samples.
         * @param[out] output The convolution result.
         * @param[in] len Number of input/output samples to process.
         */
        void Process(const AmAudioSample* input, AmAudioSample* output, AmSize len);

        /**
         * @brief Resets the convolver state and discards the set impulse response.
         *
         * The convolver will need to be @ref Init initialized again after this call.
         */
        void Reset();

        /**
         * @brief Gets the size of a single convolution segment.
         *
         * @return The size of a single convolution segment.
         */
        [[nodiscard]] AmSize GetSegmentSize() const;

        /**
         * @brief Gets the number of convolution segments.
         *
         * @return The number of convolution segments.
         */
        [[nodiscard]] AmSize GetSegmentCount() const;

    private:
        AmSize _blockSize;
        AmSize _segSize;
        AmSize _segCount;
        AmSize _fftComplexSize;
        std::vector<SplitComplex*> _segments;
        std::vector<SplitComplex*> _segmentsIR;
        AmAlignedReal32Buffer _fftBuffer;
        FFT _fft;
        SplitComplex _preMultiplied;
        SplitComplex _conv;
        AmAlignedReal32Buffer _overlap;
        AmSize _current;
        AmAlignedReal32Buffer _inputBuffer;
        AmSize _inputBufferFill;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_DSP_CONVOLVER_H
