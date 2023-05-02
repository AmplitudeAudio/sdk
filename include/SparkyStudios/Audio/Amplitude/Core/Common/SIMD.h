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

#ifndef SS_AMPLITUDE_AUDIO_SIMD_H
#define SS_AMPLITUDE_AUDIO_SIMD_H

#if defined(AM_SIMD_INTRINSICS)

#if defined(__AVX2__) || defined(SIMDPP_ARCH_X86_AVX2)
#if !defined(SIMDPP_ARCH_X86_AVX2)
#define SIMDPP_ARCH_X86_AVX2
#endif
#define AM_SIMD_ARCH_AVX2
#elif defined(__AVX__)
#if !defined(SIMDPP_ARCH_X86_AVX)
#define SIMDPP_ARCH_X86_AVX
#define AM_SIMD_ARCH_AVX
#endif
#endif

#if (defined(__SSE4_2__) || defined(__SSE4_1__) || defined(__SSE4A__)) || defined(SIMDPP_ARCH_X86_SSE4_1)
#if !defined(SIMDPP_ARCH_X86_SSE4_1)
#define SIMDPP_ARCH_X86_SSE4_1
#endif
#define AM_SIMD_ARCH_SSE4_1
#define AM_SIMD_ARCH_SSSE3
#define AM_SIMD_ARCH_SSE3
#define AM_SIMD_ARCH_SSE2
#elif defined(__SSSE3__) || defined(SIMDPP_ARCH_X86_SSSE3) || defined(SIMDPP_ARCH_X86_SSSE3)
#if !defined(SIMDPP_ARCH_X86_SSSE3)
#define SIMDPP_ARCH_X86_SSSE3
#endif
#define AM_SIMD_ARCH_SSSE3
#define AM_SIMD_ARCH_SSE3
#define AM_SIMD_ARCH_SSE2
#elif defined(__SSE3__) || defined(SIMDPP_ARCH_X86_SSE3)
#if !defined(SIMDPP_ARCH_X86_SSE3)
#define SIMDPP_ARCH_X86_SSE3
#endif
#define AM_SIMD_ARCH_SSE3
#define AM_SIMD_ARCH_SSE2
#elif defined(__SSE2__) || defined(SIMDPP_ARCH_X86_SSE2)
#if !defined(SIMDPP_ARCH_X86_SSE2)
#define SIMDPP_ARCH_X86_SSE2
#endif
#define AM_SIMD_ARCH_SSE2
#endif

#if defined(__FMA__) || defined(SIMDPP_ARCH_X86_FMA3)
#if !defined(SIMDPP_ARCH_X86_FMA3)
#define SIMDPP_ARCH_X86_FMA3
#endif
#define AM_SIMD_ARCH_FMA3
#endif

#if defined(AM_CPU_ARM_NEON) || defined(SIMDPP_ARCH_ARM_NEON)
#if !defined(SIMDPP_ARCH_ARM_NEON)
#define SIMDPP_ARCH_ARM_NEON
#endif
#define AM_SIMD_ARCH_NEON
#endif

#if defined(AM_SIMD_ARCH_AVX2)
#define MA_SUPPORT_AVX2
#define MA_PREFER_AVX2
#elif defined(AM_SIMD_ARCH_SSE2)
#define MA_SUPPORT_SSE2
#define MA_PREFER_SSE2
#elif defined(AM_SIMD_ARCH_NEON)
#define MA_SUPPORT_NEON
#define MA_PREFER_NEON
#endif

#endif

#endif // SS_AMPLITUDE_AUDIO_SIMD_H