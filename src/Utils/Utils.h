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

#ifndef _AM_IMPLEMENTATION_UTILS_UTILS_H
#define _AM_IMPLEMENTATION_UTILS_UTILS_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>
#include <SparkyStudios/Audio/Amplitude/Math/SplitComplex.h>
#include <SparkyStudios/Audio/Amplitude/Math/Utils.h>

#if defined(AM_SIMD_INTRINSICS)
#include <xsimd/xsimd.hpp>
#endif // defined(AM_SIMD_INTRINSICS)

namespace SparkyStudios::Audio::Amplitude
{
#if defined(AM_SIMD_INTRINSICS)
    typedef xsimd::batch<AmReal32, xsimd::best_arch> AmAudioFrame;
#else
    typedef AmReal32 AmAudioFrame;
#endif // AM_SIMD_INTRINSICS

    typedef AmAudioFrame* AmAudioFrameBuffer;

    AM_INLINE constexpr AmSize GetSimdBlockSize()
    {
#if defined(AM_SIMD_INTRINSICS)
        return AmAudioFrame::size;
#else
        return 1;
#endif // AM_SIMD_INTRINSICS
    }

    AM_INLINE AmSize GetNumChunks(AmSize size, AmSize blockSize)
    {
        return size - size % blockSize;
    }

    AM_INLINE AmSize GetNumSimdChunks(AmSize size)
    {
        constexpr AmSize blockSize = GetSimdBlockSize();

        return GetNumChunks(size, blockSize);
    }

    AM_INLINE void Sum(
        AmAudioSample* AM_RESTRICT result, const AmAudioSample* AM_RESTRICT a, const AmAudioSample* AM_RESTRICT b, const AmSize len)
    {
#if defined(AM_SIMD_INTRINSICS)
        const AmSize end = GetNumSimdChunks(len);
        constexpr AmSize blockSize = GetSimdBlockSize();

        for (AmSize i = 0; i < end; i += blockSize)
        {
            const auto ba = xsimd::load_aligned(&a[i]);
            const auto bb = xsimd::load_aligned(&b[i]);

            auto res = xsimd::add(ba, bb);
            res.store_aligned(&result[i]);
        }
#else
        const AmSize end = GetNumChunks(len, 4);

        for (AmSize i = 0; i < end; i += 4)
        {
            result[i + 0] = a[i + 0] + b[i + 0];
            result[i + 1] = a[i + 1] + b[i + 1];
            result[i + 2] = a[i + 2] + b[i + 2];
            result[i + 3] = a[i + 3] + b[i + 3];
        }
#endif

        for (AmSize i = end; i < len; ++i)
        {
            result[i] = a[i] + b[i];
        }
    }

    AM_INLINE void ComplexMultiplyAccumulate(
        AmAudioSample* AM_RESTRICT re,
        AmAudioSample* AM_RESTRICT im,
        const AmAudioSample* AM_RESTRICT reA,
        const AmAudioSample* AM_RESTRICT imA,
        const AmAudioSample* AM_RESTRICT reB,
        const AmAudioSample* AM_RESTRICT imB,
        const AmSize len)
    {
#if defined(AM_SIMD_INTRINSICS)
        const AmSize end = GetNumSimdChunks(len);
        constexpr AmSize blockSize = GetSimdBlockSize();

        for (AmSize i = 0; i < end; i += blockSize)
        {
            const auto ra = xsimd::load_aligned(&reA[i]);
            const auto rb = xsimd::load_aligned(&reB[i]);
            const auto ia = xsimd::load_aligned(&imA[i]);
            const auto ib = xsimd::load_aligned(&imB[i]);

            auto real = xsimd::load_aligned(&re[i]);
            auto imag = xsimd::load_aligned(&im[i]);

            real = xsimd::fma(ra, rb, real);
            real = xsimd::sub(real, xsimd::mul(ia, ib));
            real.store_aligned(&re[i]);

            imag = xsimd::fma(ra, ib, imag);
            imag = xsimd::fma(ia, rb, imag);
            imag.store_aligned(&im[i]);
        }
#else
        const AmSize end = GetNumChunks(len, 4);

        for (AmSize i = 0; i < end; i += 4)
        {
            re[i + 0] += reA[i + 0] * reB[i + 0] - imA[i + 0] * imB[i + 0];
            re[i + 1] += reA[i + 1] * reB[i + 1] - imA[i + 1] * imB[i + 1];
            re[i + 2] += reA[i + 2] * reB[i + 2] - imA[i + 2] * imB[i + 2];
            re[i + 3] += reA[i + 3] * reB[i + 3] - imA[i + 3] * imB[i + 3];

            im[i + 0] += reA[i + 0] * imB[i + 0] + imA[i + 0] * reB[i + 0];
            im[i + 1] += reA[i + 1] * imB[i + 1] + imA[i + 1] * reB[i + 1];
            im[i + 2] += reA[i + 2] * imB[i + 2] + imA[i + 2] * reB[i + 2];
            im[i + 3] += reA[i + 3] * imB[i + 3] + imA[i + 3] * reB[i + 3];
        }
#endif

        for (AmSize i = end; i < len; ++i)
        {
            re[i] += reA[i] * reB[i] - imA[i] * imB[i];
            im[i] += reA[i] * imB[i] + imA[i] * reB[i];
        }
    }

    AM_INLINE void ComplexMultiplyAccumulate(SplitComplex& result, const SplitComplex& a, const SplitComplex& b)
    {
        AMPLITUDE_ASSERT(result.GetSize() == a.GetSize());
        AMPLITUDE_ASSERT(result.GetSize() == b.GetSize());

        ComplexMultiplyAccumulate(result.re(), result.im(), a.re(), a.im(), b.re(), b.im(), result.GetSize());
    }

    /**
     * @brief Copies a source array into a destination buffer and pads the destination buffer with zeros.
     *
     * @param dest The destination buffer.
     * @param src The source array.
     * @param srcSize The size of the source array.
     */
    AM_INLINE void CopyAndPad(AmAlignedReal32Buffer& dest, const AmReal32* src, AmSize srcSize)
    {
        AMPLITUDE_ASSERT(dest.GetSize() >= srcSize);

        std::memcpy(dest.GetBuffer(), src, srcSize * sizeof(AmReal32));
        std::memset(dest.GetBuffer() + srcSize, 0, (dest.GetSize() - srcSize) * sizeof(AmReal32));
    }

    /**
     * @brief Gets the number of B-Format components for the given order and 3D status.
     *
     * @param order The ambisonic order.
     * @param is3D Whether the ambisonic signal is 3D (true) or 2D (false).
     *
     * @return The number of B-Format components for the given order and 3D status.
     */
    AM_INLINE AmUInt32 OrderToComponents(const AmUInt32 order, const bool is3D)
    {
        return is3D ? static_cast<AmUInt32>(std::pow(static_cast<AmReal32>(order) + 1.f, 2.f)) : order * 2 + 1;
    }

    template<typename T>
    AM_INLINE AmSize FindNextAlignedArrayIndex(AmSize length, AmSize memoryAlignmentBytes)
    {
        constexpr AmSize sizeOfT = sizeof(T);

        const AmSize size = sizeOfT * length;
        const AmSize unalignedSize = size % memoryAlignmentBytes;
        const AmSize bytesToNextAligned = unalignedSize == 0 ? 0 : memoryAlignmentBytes - unalignedSize;

        return (size + bytesToNextAligned) / sizeOfT;
    }

    AM_INLINE void Deinterleave(
        const AmReal32* in, AmUInt64 inOffset, AmReal32* out, AmUInt64 outOffset, AmInt32 numSamples, AmInt32 numChannels)
    {
        if (numChannels == 1)
        {
            std::memcpy(out + outOffset, in + inOffset, numSamples * sizeof(AmReal32));
            return;
        }

        for (AmInt32 i = 0; i < numSamples; ++i)
            for (AmInt32 j = 0; j < numChannels; ++j)
                out[j * numSamples + i + outOffset] = in[(i + inOffset) * numChannels + j];
    }

    void Interleave(const AudioBuffer* in, AmUInt64 inOffset, AmReal32* out, AmUInt64 outOffset, AmInt32 numSamples, AmInt32 numChannels);

    AM_INLINE void ScalarMultiply(const AmReal32* input, AmReal32* output, AmReal32 scalar, AmSize length)
    {
        AmSize remaining = length;

#if defined(AM_SIMD_INTRINSICS)
        const AmSize end = GetNumSimdChunks(length);
        constexpr AmSize blockSize = GetSimdBlockSize();
        remaining = remaining - end;

        const auto& bb = xsimd::batch(scalar);

        for (AmSize i = 0; i < end; i += blockSize)
        {
            auto ba = xsimd::load_aligned(input + i);

            auto res = xsimd::mul(ba, bb);
            res.store_aligned(output + i);
        }
#endif

        for (AmSize i = length - remaining; i < length; i++)
            output[i] = input[i] * scalar;
    }

    AM_INLINE void PointwiseMultiply(const AmReal32* inputA, const AmReal32* inputB, AmReal32* output, AmSize length)
    {
        AmSize remaining = length;

#if defined(AM_SIMD_INTRINSICS)
        const AmSize end = GetNumSimdChunks(length);
        constexpr AmSize blockSize = GetSimdBlockSize();
        remaining = remaining - end;

        for (AmSize i = 0; i < end; i += blockSize)
        {
            const auto ba = xsimd::load_aligned(inputA + i);
            const auto bb = xsimd::load_aligned(inputB + i);

            auto res = xsimd::mul(ba, bb);
            res.store_aligned(output + i);
        }
#endif

        for (AmSize i = length - remaining; i < length; i++)
            output[i] = inputA[i] * inputB[i];
    }

    AM_INLINE void GenerateHannWindow(bool fullWindow, AmSize windowLength, AudioBufferChannel* buffer)
    {
        AMPLITUDE_ASSERT(buffer != nullptr);
        AMPLITUDE_ASSERT(windowLength <= buffer->size());

        const AmReal32 fullWindowScalingFactor = (2.0f * AM_PI32) / (static_cast<AmReal32>(windowLength) - 1.0f);
        const AmReal32 halfWindowScalingFactor = (2.0f * AM_PI32) / (2.0f * static_cast<AmReal32>(windowLength) - 1.0f);
        const AmReal32 scalingFactor = (fullWindow) ? fullWindowScalingFactor : halfWindowScalingFactor;

        for (AmSize i = 0; i < windowLength; ++i)
            (*buffer)[i] = 0.5f * (1.0f - std::cos(scalingFactor * static_cast<AmReal32>(i)));
    }
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_UTILS_UTILS_H
