// Copyright (c) 2023-present Sparky Studios. All rights reserved.
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

#ifndef SS_AMPLITUDE_COMMON_H
#define SS_AMPLITUDE_COMMON_H

#if defined(_WIN32) || defined(_WIN64) || defined(WINAPI_FAMILY)
typedef wchar_t am_oschar;
#elif defined(__ANDROID__)
typedef char am_oschar;
#elif defined(__linux__)
typedef char am_oschar;
#elif defined(__APPLE__)
typedef char am_oschar;
#endif

typedef size_t am_size;
typedef signed char am_int8;
typedef unsigned char am_uint8;
typedef signed short am_int16;
typedef unsigned short am_uint16;
typedef signed int am_int32;
typedef unsigned int am_uint32;
typedef float am_float32;
typedef double am_float64;
typedef am_float64 am_time;
#if defined(_MSC_VER) && !defined(__clang__)
typedef signed __int64 am_int64;
typedef unsigned __int64 am_uint64;
#else
#if defined(__clang__) || (defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)))
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wlong-long"
#if defined(__clang__)
#pragma GCC diagnostic ignored "-Wc++11-long-long"
#endif
#endif
typedef signed long long am_int64;
typedef unsigned long long am_uint64;
#if defined(__clang__) || (defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)))
#pragma GCC diagnostic pop
#endif
#endif

typedef void* am_voidptr;

typedef am_uint32 am_bool;
#define AM_TRUE 1
#define AM_FALSE 0

#include <SparkyStudios/Audio/Amplitude/Math/HandmadeMath.h>

typedef AmVec2 am_vec2;
typedef AmVec3 am_vec3;
typedef AmVec4 am_vec4;

#endif // SS_AMPLITUDE_COMMON_H
