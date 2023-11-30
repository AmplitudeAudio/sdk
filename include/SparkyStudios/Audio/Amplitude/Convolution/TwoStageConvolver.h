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
// Based on the code from https://github.com/HiFi-LoFi/Convolver
// Copyright (c) 2017 HiFi-LoFi, MIT License

#pragma once

#ifndef SS_AMPLITUDE_AUDIO_CONVOLUTION_TWO_STAGE_CONVOLVER_H
#define SS_AMPLITUDE_AUDIO_CONVOLUTION_TWO_STAGE_CONVOLVER_H

#include <SparkyStudios/Audio/Amplitude/Convolution/Convolver.h>

namespace SparkyStudios::Audio::Amplitude::Convolution
{
    /**
     * @class TwoStageConvolver
     * @brief FFT convolver using two different block sizes
     *
     * The 2-stage convolver consists internally of two convolvers:
     *
     * - A head convolver, which processes the only the begin of the impulse response.
     *
     * - A tail convolver, which processes the rest and major amount of the impulse response.
     *
     * Using a short block size for the head convolver and a long block size for
     * the tail convolver results in much less CPU usage, while keeping the
     * calculation time of each processing call short.
     *
     * Furthermore, this convolver class provides virtual methods which provide the
     * possibility to move the tail convolution into the background (e.g. by using
     * multithreading, see StartBackgroundProcessing()/WaitForBackgroundProcessing()).
     *
     * As well as the basic Convolver class, the 2-stage convolver is suitable
     * for real-time processing which means that no "unpredictable" operations like
     * allocations, locking, API calls, etc. are performed during processing (all
     * necessary allocations and preparations take place during initialization).
     */
    class TwoStageConvolver
    {
    public:
        TwoStageConvolver();
        virtual ~TwoStageConvolver();

        // Prevent uncontrolled usage
        TwoStageConvolver(const TwoStageConvolver&) = delete;
        TwoStageConvolver& operator=(const TwoStageConvolver&) = delete;

        /**
         * @brief Initialization the convolver.
         *
         * @param headBlockSize The head block size
         * @param tailBlockSize the tail block size
         * @param ir The impulse response
         * @param irLen Length of the impulse response in samples
         *
         * @return @c true on success, @c false otherwise.
         */
        bool Init(size_t headBlockSize, size_t tailBlockSize, const AmAudioSample* ir, size_t irLen);

        /**
         * @brief Convolves the the given input samples and immediately outputs the result
         *
         * @param input The input samples
         * @param output The convolution result
         * @param len Number of input/output samples
         */
        void Process(const AmAudioSample* input, AmAudioSample* output, size_t len);

        /**
         * @brief Resets the convolver and discards the set impulse response
         */
        void Reset();

    protected:
        /**
         * @brief Method called by the convolver if work for background processing is available
         *
         * The default implementation just calls DoBackgroundProcessing() to perform the "bulk"
         * convolution. However, if you want to perform the majority of work in some background
         * thread (which is recommended), you can overload this method and trigger the execution
         * of DoBackgroundProcessing() really in some background thread.
         */
        virtual void StartBackgroundProcessing();

        /**
         * @brief Called by the convolver if it expects the result of its previous call to startBackgroundProcessing()
         *
         * After returning from this method, all background processing has to be completed.
         */
        virtual void WaitForBackgroundProcessing();

        /**
         * @brief Actually performs the background processing work
         */
        void DoBackgroundProcessing();

    private:
        size_t _headBlockSize;
        size_t _tailBlockSize;
        Convolver _headConvolver;
        Convolver _tailConvolver0;
        AmAlignedReal32Buffer _tailOutput0;
        AmAlignedReal32Buffer _tailPrecalculated0;
        Convolver _tailConvolver;
        AmAlignedReal32Buffer _tailOutput;
        AmAlignedReal32Buffer _tailPrecalculated;
        AmAlignedReal32Buffer _tailInput;
        size_t _tailInputFill;
        size_t _precalculatedPos;
        AmAlignedReal32Buffer _backgroundProcessingInput;
    };
} // namespace SparkyStudios::Audio::Amplitude::Convolution

#endif // SS_AMPLITUDE_AUDIO_CONVOLUTION_TWO_STAGE_CONVOLVER_H