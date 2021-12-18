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

#if defined(__AVX2__)
#define SIMDPP_ARCH_X86_AVX2
#elif defined(__AVX__)
#define SIMDPP_ARCH_X86_AVX
#endif

#if defined(__SSE4_2__) || defined(__SSE4_1__) || defined(__SSE4A__)
#define SIMDPP_ARCH_X86_SSE4_1
#elif defined(__SSSE3__)
#define SIMDPP_ARCH_X86_SSSE3
#elif defined(__SSE3__)
#define SIMDPP_ARCH_X86_SSE3
#elif defined(__SSE2__)
#define SIMDPP_ARCH_X86_SSE2
#endif

#if defined(__FMA__)
#define SIMDPP_ARCH_X86_FMA3
#endif

#if defined(AM_CPU_ARM_NEON)
#define SIMDPP_ARCH_ARM_NEON
#endif

#endif // SS_AMPLITUDE_AUDIO_SIMD_H