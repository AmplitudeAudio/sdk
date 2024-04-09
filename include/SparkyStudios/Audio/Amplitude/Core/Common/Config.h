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

#ifndef SS_AMPLITUDE_AUDIO_CONFIG_H
#define SS_AMPLITUDE_AUDIO_CONFIG_H

#if defined(__GNUC__) || defined(__clang__)
#define AM_DEPRECATED(msg) __attribute__((deprecated(msg)))
#elif defined(_MSC_VER)
#define AM_DEPRECATED(msg) __declspec(deprecated(msg))
#else
#define AM_DEPRECATED(msg)
#endif

#if defined(_WIN32) || defined(_WIN64) || defined(WINAPI_FAMILY)
#include <SparkyStudios/Audio/Amplitude/Core/Common/Platforms/Windows/Config.h>
#elif defined(__ANDROID__)
#include <SparkyStudios/Audio/Amplitude/Core/Common/Platforms/Android/Config.h>
#elif defined(__linux__)
#include <SparkyStudios/Audio/Amplitude/Core/Common/Platforms/Linux/Config.h>
#elif defined(__APPLE__)
#include <SparkyStudios/Audio/Amplitude/Core/Common/Platforms/Apple/Config.h>
#endif

#if !defined(AM_VALUE_ALIGN)
#define AM_VALUE_ALIGN(_value_, _alignment_) (((_value_) + ((_alignment_)-1)) & ~((_alignment_)-1))
#endif

#if !defined(AM_CALL_POLICY)
#define AM_CALL_POLICY
#endif

#if defined(AM_BUILDSYSTEM_SHARED)
#if defined(AM_BUILDSYSTEM_BUILDING_AMPLITUDE)
#define AM_API_PUBLIC AM_LIB_EXPORT
#else
#define AM_API_PUBLIC AM_LIB_IMPORT
#endif
#define AM_API_PRIVATE AM_LIB_PRIVATE
#if defined(AM_BUILDSYSTEM_BUILDING_PLUGIN)
#define AM_API_PLUGIN AM_LIB_EXPORT
#else
#define AM_API_PLUGIN
#endif
#else // AM_BUILDSYSTEM_STATIC
#define AM_API_PUBLIC
#define AM_API_PRIVATE static
#define AM_API_PLUGIN
#endif

#if !defined(AM_RESTRICT)
#define AM_RESTRICT
#endif

#if !defined(AMPLITUDE_DISABLE_SIMD)
#if defined(AM_CPU_X86) || defined(AM_CPU_X86_64) || defined(AM_CPU_ARM) || defined(AM_CPU_ARM_64)
#define AM_SIMD_INTRINSICS
#include <SparkyStudios/Audio/Amplitude/Core/Common/SIMD.h>
#endif // AM_CPU_X86 || AM_CPU_X86_64 || AM_CPU_ARM_NEON
#else
#define PFFFT_SIMD_DISABLE
#define HANDMADE_MATH_NO_SIMD
#define MA_NO_NEON
#define MA_NO_AVX2
#define MA_NO_AVX
#define MA_NO_SSE2
#endif // AMPLITUDE_DISABLE_SIMD

// Define the value of Pi if the platform doesn't do that
#ifndef M_PI
#define M_PI 3.14159265358979323846264f // from CRC
#endif

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
// Configuration defines

// Maximum number of filters per stream
#define AM_FILTERS_PER_STREAM 8

// 1) Mono, 2) Stereo, 4) Quad, 6) 5.1, 8) 7.1
#define AM_MAX_CHANNELS 8

// Maximum number of tasks in a single pool
#define AM_MAX_THREAD_POOL_TASKS 1024

#endif // SS_AMPLITUDE_AUDIO_CONFIG_H
