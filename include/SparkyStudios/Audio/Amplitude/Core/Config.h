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

#include <cmath> // sin
#include <cstdlib> // rand

#ifdef AMPLITUDE_NO_ASSERTS
#define AMPLITUDE_ASSERT(x)
#else
#ifdef _MSC_VER
#include <cstdio> // for sprintf in asserts
#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h> // only needed for OutputDebugStringA, should be solved somehow.
#define AMPLITUDE_ASSERT(x)                                                                                                                \
    if (!(x))                                                                                                                              \
    {                                                                                                                                      \
        char temp[200];                                                                                                                    \
        sprintf(temp, "%s(%d): assert(%s) failed.\n", __FILE__, __LINE__, #x);                                                             \
        OutputDebugStringA(temp);                                                                                                          \
        __debugbreak();                                                                                                                    \
    }
#else
#include <cassert> // assert
#define AMPLITUDE_ASSERT(x) assert(x)
#endif
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846264f // from CRC
#endif

#if defined(_WIN32) || defined(_WIN64)
#define WINDOWS_VERSION
#endif

#if !defined(DISABLE_SIMD)
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386) || defined(_M_IX86)
#define AMPLITUDE_SSE_INTRINSICS
#endif
#endif

#define AMPLITUDE_VERSION 202002

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
// Configuration defines

// Maximum number of filters per stream
#define FILTERS_PER_STREAM 8

// 1)mono, 2)stereo 4)quad 6)5.1 8)7.1
#define MAX_CHANNELS 8

// Default resampler for both main and GetBus mixers
#define AMPLITUDE_DEFAULT_RESAMPLER SoLoud::Engine::RESAMPLER_LINEAR

#endif // SS_AMPLITUDE_AUDIO_CONFIG_H
