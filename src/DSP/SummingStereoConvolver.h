// Copyright (c) 2026-present Sparky Studios. All rights reserved.
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

#ifndef _AM_IMPLEMENTATION_DSP_SUMMING_STEREO_CONVOLVER_H
#define _AM_IMPLEMENTATION_DSP_SUMMING_STEREO_CONVOLVER_H

#include <memory>
#include <span>
#include <vector>

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>
#include <SparkyStudios/Audio/Amplitude/DSP/FFT.h>
#include <SparkyStudios/Audio/Amplitude/DSP/SplitComplex.h>

#include <DSP/ConvolverKernel.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief Convolves N mono inputs with N left and N right kernels and sums them into two outputs.
     *
     * The sum happens in the frequency domain: each call runs one forward FFT per input and one inverse FFT per ear.
     * Block semantics match Convolver: zero latency, and partial blocks are buffered until the block fills.
     */
    class SummingStereoConvolver
    {
    public:
        static constexpr AmSize kMaxInputs = 16;

        SummingStereoConvolver() = default;
        ~SummingStereoConvolver();

        SummingStereoConvolver(const SummingStereoConvolver& other) = delete;
        SummingStereoConvolver& operator=(const SummingStereoConvolver& other) = delete;

        /**
         * @brief Sets the kernels and allocates every buffer Process needs.
         *
         * @param left The left-ear kernel of each input.
         * @param right The right-ear kernel of each input.
         *
         * @return false when the counts differ, are 0 or above kMaxInputs, a kernel is null, or non-empty kernels have
         * different block sizes. A failed Init leaves the convolver empty.
         */
        bool Init(std::span<const std::shared_ptr<const ConvolverKernel>> left, std::span<const std::shared_ptr<const ConvolverKernel>> right);

        /**
         * @brief Adds the convolution of the inputs into the outputs. Never allocates.
         *
         * @param inputs GetInputCount() input pointers, each with len samples.
         * @param outL The left output, added to.
         * @param outR The right output, added to.
         * @param len The number of samples.
         */
        void Process(const AmAudioSample* const* inputs, AmAudioSample* outL, AmAudioSample* outR, AmSize len);

        /**
         * @brief Clears the history, the buffered input and the overlap. The kernels are kept.
         */
        void Reset();

        [[nodiscard]] AmSize GetInputCount() const;

    private:
        void Release();

        SplitComplex& History(AmSize input, AmSize slot);

        std::vector<std::shared_ptr<const ConvolverKernel>> _kernels[2];
        AmSize _inputCount = 0;
        AmSize _blockSize = 0;
        AmSize _complexSize = 0;
        AmSize _ringSize = 0;

        FFT _fft;
        AmAlignedReal32Buffer _fftBuffer;
        std::vector<SplitComplex*> _history;
        AmAlignedReal32Buffer _inputBuffer;
        SplitComplex _preMultiplied[2];
        SplitComplex _conv[2];
        AmAlignedReal32Buffer _overlap[2];
        AmSize _current = 0;
        AmSize _inputBufferFill = 0;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_DSP_SUMMING_STEREO_CONVOLVER_H
