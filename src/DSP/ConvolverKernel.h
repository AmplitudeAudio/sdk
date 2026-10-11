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

#ifndef _AM_IMPLEMENTATION_DSP_CONVOLVER_KERNEL_H
#define _AM_IMPLEMENTATION_DSP_CONVOLVER_KERNEL_H

#include <memory>
#include <vector>

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>
#include <SparkyStudios/Audio/Amplitude/DSP/SplitComplex.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief Immutable, uniformly partitioned frequency-domain kernel of one impulse response.
     *
     * Built once on the game thread and shared, read-only, by every convolver that uses the same impulse response.
     */
    class ConvolverKernel
    {
    public:
        /**
         * @brief Builds the kernel of an impulse response.
         *
         * Trailing samples under 1e-6 are trimmed, the block size is rounded up to a power of two, and each block is
         * zero-padded to twice its size and forward-transformed.
         *
         * @param blockSize The partition size before rounding.
         * @param ir The impulse response.
         * @param irLen The impulse response length.
         *
         * @return The kernel, or nullptr when blockSize is 0. An all-zero impulse response gives a kernel with no
         * segment.
         */
        static std::shared_ptr<const ConvolverKernel> Create(AmSize blockSize, const AmAudioSample* ir, AmSize irLen);

        ConvolverKernel() = default;
        ~ConvolverKernel();

        ConvolverKernel(const ConvolverKernel& other) = delete;
        ConvolverKernel& operator=(const ConvolverKernel& other) = delete;

        [[nodiscard]] AmSize GetBlockSize() const;
        [[nodiscard]] AmSize GetSegmentSize() const;
        [[nodiscard]] AmSize GetSegmentCount() const;
        [[nodiscard]] AmSize GetComplexSize() const;
        [[nodiscard]] const SplitComplex& GetSegment(AmSize index) const;

    private:
        AmSize _blockSize = 0;
        AmSize _segmentSize = 0;
        AmSize _complexSize = 0;
        std::vector<SplitComplex*> _segments;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_DSP_CONVOLVER_KERNEL_H
