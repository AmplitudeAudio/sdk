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
#include <SparkyStudios/Audio/Amplitude/Math/Utils.h>

#if defined(AM_SIMD_INTRINSICS)
#include <simdpp/simd.h>
#endif // defined(AM_SIMD_INTRINSICS)

namespace SparkyStudios::Audio::Amplitude
{
#if defined(AM_SIMD_INTRINSICS)
    typedef simdpp::float32v AmAudioFrame;
#else
    typedef AmReal32 AmAudioFrame;
#endif // AM_SIMD_INTRINSICS

    typedef AmAudioFrame* AmAudioFrameBuffer;
} // namespace SparkyStudios::Audio::Amplitude

#if defined(AM_SIMD_INTRINSICS)

namespace simdpp
{
    namespace SIMDPP_ARCH_NAMESPACE
    {
        template<unsigned N, class V1, class V2>
        SIMDPP_INL typename detail::get_expr2_nomask<V1, V2>::empty zip_lo(const any_vec16<N, V1>& a, const any_vec16<N, V2>& b)
        {
            return zip8_lo(a, b);
        }

        template<unsigned N, class V1, class V2>
        SIMDPP_INL typename detail::get_expr2_nomask<V1, V2>::empty zip_hi(const any_vec16<N, V1>& a, const any_vec16<N, V2>& b)
        {
            return zip8_hi(a, b);
        }
    } // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif // AM_SIMD_INTRINSICS

#endif // SS_AMPLITUDE_AUDIO_UTILS_H
