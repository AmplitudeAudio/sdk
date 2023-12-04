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

#ifndef SS_AMPLITUDE_AUDIO_CONVOLUTION_CONVOLVER_H
#define SS_AMPLITUDE_AUDIO_CONVOLUTION_CONVOLVER_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

#include <SparkyStudios/Audio/Amplitude/Math/FFT.h>

namespace SparkyStudios::Audio::Amplitude::Convolution
{
    /**
     * @class Convolver
     * @brief Implementation of a partitioned FFT convolution algorithm with uniform block size
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
     */
    class Convolver
    {
    public:
        Convolver();
        virtual ~Convolver();

        // Prevent uncontrolled usage
        Convolver(const Convolver&) = delete;
        Convolver& operator=(const Convolver&) = delete;

        /**
         * @brief Initializes the convolver
         *
         * @param blockSize Block size internally used by the convolver (partition size)
         * @param ir The impulse response
         * @param irLen Length of the impulse response
         *
         * @return true: Success - false: Failed
         */
        bool Init(AmSize blockSize, const AmAudioSample* ir, AmSize irLen);

        /**
         * @brief Convolves the the given input samples and immediately outputs the result
         * @param input The input samples
         * @param output The convolution result
         * @param len Number of input/output samples
         */
        void Process(const AmAudioSample* input, AmAudioSample* output, AmSize len);

        /**
         * @brief Resets the convolver and discards the set impulse response
         */
        void Reset();

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
} // namespace SparkyStudios::Audio::Amplitude::Convolution

#endif // SS_AMPLITUDE_AUDIO_CONVOLUTION_CONVOLVER_H