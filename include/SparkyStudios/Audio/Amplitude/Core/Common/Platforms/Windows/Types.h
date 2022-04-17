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

#ifndef SS_AMPLITUDE_AUDIO_WINDOWS_TYPES_H
#define SS_AMPLITUDE_AUDIO_WINDOWS_TYPES_H

#if defined(_MSC_VER) && _MSC_VER < 1600
    typedef unsigned __int64        uint64_t;
    typedef unsigned __int32        uint32_t;
    typedef unsigned __int16        uint16_t;
    typedef unsigned __int8         uint8_t;
    typedef __int64                 int64_t;
    typedef __int32                 int32_t;
    typedef __int16                 int16_t;
    typedef __int8                  int8_t;
#else
#include <cstdint>
#endif

#include <string>

namespace SparkyStudios::Audio::Amplitude
{
    typedef int8_t                  AmInt8;
    typedef int16_t                 AmInt16;
    typedef int32_t                 AmInt32;
    typedef int64_t                 AmInt64;

    typedef uint8_t                 AmUInt8;
    typedef uint16_t                AmUInt16;
    typedef uint32_t                AmUInt32;
    typedef uint64_t                AmUInt64;

    typedef size_t                  AmSize;

    typedef float                   AmReal32;
    typedef double                  AmReal64;

    typedef int8_t*                 AmInt8Buffer;
    typedef int16_t*                AmInt16Buffer;
    typedef int32_t*                AmInt32Buffer;
    typedef int64_t*                AmInt64Buffer;

    typedef uint8_t*                AmUInt8Buffer;
    typedef uint16_t*               AmUInt16Buffer;
    typedef uint32_t*               AmUInt32Buffer;
    typedef uint64_t*               AmUInt64Buffer;

    typedef float*                  AmReal32Buffer;
    typedef double*                 AmReal64Buffer;

    typedef void*                   AmVoidPtr;

    typedef const int8_t*           AmConstInt8Buffer;
    typedef const int16_t*          AmConstInt16Buffer;
    typedef const int32_t*          AmConstInt32Buffer;
    typedef const int64_t*          AmConstInt64Buffer;

    typedef const uint8_t*          AmConstUInt8Buffer;
    typedef const uint16_t*         AmConstUInt16Buffer;
    typedef const uint32_t*         AmConstUInt32Buffer;
    typedef const uint64_t*         AmConstUInt64Buffer;

    typedef const float*            AmConstReal32Buffer;
    typedef const double*           AmConstReal64Buffer;

    typedef const void*             AmConstVoidPtr;

    typedef intptr_t                AmIntPtr;
    typedef uintptr_t               AmUIntPtr;

    typedef std::string             AmString;
    typedef wchar_t                 AmOsChar;
    typedef std::wstring            AmOsString;

    typedef FILE*                   AmFileHandle;
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_WINDOWS_TYPES_H
