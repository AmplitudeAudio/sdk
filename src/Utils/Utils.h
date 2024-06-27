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

#ifndef SS_AMPLITUDE_AUDIO_UTILS_H
#define SS_AMPLITUDE_AUDIO_UTILS_H

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

    AM_INLINE(void)
    Sum(AmAudioSample* AM_RESTRICT result, const AmAudioSample* AM_RESTRICT a, const AmAudioSample* AM_RESTRICT b, const AmSize len)
    {
#if defined(AM_SIMD_INTRINSICS)
        const AmSize end = AmAudioFrame::size * (len / AmAudioFrame::size);

        for (AmSize i = 0; i < end; i += AmAudioFrame::size)
        {
            const auto ba = xsimd::load_aligned(&a[i]);
            const auto bb = xsimd::load_aligned(&b[i]);

            auto res = xsimd::add(ba, bb);
            res.store_aligned(&result[i]);
        }

        for (AmSize i = end; i < len; i++)
        {
            result[i] = a[i] + b[i];
        }
#else
        const AmSize end4 = 4 * (len / 4);

        for (AmSize i = 0; i < end4; i += 4)
        {
            result[i + 0] = a[i + 0] + b[i + 0];
            result[i + 1] = a[i + 1] + b[i + 1];
            result[i + 2] = a[i + 2] + b[i + 2];
            result[i + 3] = a[i + 3] + b[i + 3];
        }

        for (AmSize i = end4; i < len; ++i)
        {
            result[i] = a[i] + b[i];
        }
#endif
    }

    AM_INLINE(void)
    ComplexMultiplyAccumulate(
        AmAudioSample* AM_RESTRICT re,
        AmAudioSample* AM_RESTRICT im,
        const AmAudioSample* AM_RESTRICT reA,
        const AmAudioSample* AM_RESTRICT imA,
        const AmAudioSample* AM_RESTRICT reB,
        const AmAudioSample* AM_RESTRICT imB,
        const AmSize len)
    {
#if defined(AM_SIMD_INTRINSICS)
        const AmSize end4 = AmAudioFrame::size * (len / AmAudioFrame::size);

        for (AmSize i = 0; i < end4; i += AmAudioFrame::size)
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

        for (AmSize i = end4; i < len; ++i)
        {
            re[i] += reA[i] * reB[i] - imA[i] * imB[i];
            im[i] += reA[i] * imB[i] + imA[i] * reB[i];
        }
#else
        const AmSize end4 = 4 * (len / 4);

        for (AmSize i = 0; i < end4; i += 4)
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

        for (AmSize i = end4; i < len; ++i)
        {
            re[i] += reA[i] * reB[i] - imA[i] * imB[i];
            im[i] += reA[i] * imB[i] + imA[i] * reB[i];
        }
#endif
    }

    AM_INLINE(void) ComplexMultiplyAccumulate(SplitComplex& result, const SplitComplex& a, const SplitComplex& b)
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
    AM_INLINE(void) CopyAndPad(AmAlignedReal32Buffer& dest, const AmReal32* src, AmSize srcSize)
    {
        AMPLITUDE_ASSERT(dest.GetSize() >= srcSize);

        std::memcpy(dest.GetBuffer(), src, srcSize * sizeof(AmReal32));
        std::memset(dest.GetBuffer() + srcSize, 0, (dest.GetSize() - srcSize) * sizeof(AmReal32));
    }

    AM_INLINE(AmVec3) SphericalToCartesian(AmReal32 azimuth, AmReal32 elevation, AmReal32 radius)
    {
        // Translates spherical to cartesian, where Y - up, Z - forward, X - right
        const AmReal32 x = +radius * std::cos(elevation) * std::cos(azimuth);
        const AmReal32 y = +radius * std::sin(elevation);
        const AmReal32 z = -radius * std::cos(elevation) * std::sin(azimuth);

        return AM_V3(x, y, z);
    }
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_UTILS_H
