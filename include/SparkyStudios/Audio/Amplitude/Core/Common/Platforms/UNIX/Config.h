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

#ifndef SS_AMPLITUDE_AUDIO_UNIX_CONFIG_H
#define SS_AMPLITUDE_AUDIO_UNIX_CONFIG_H

// Function inline
#define AM_INLINE inline
#define AM_NO_INLINE __attribute__((noinline))

// Alignment required for SIMD data processing
#include <cstdlib>
#include <malloc.h>
#include <string.h>

static AM_INLINE(void*) am_aligned_realloc(void* ptr, size_t alignment, size_t size)
{
    if (!ptr)
        return std::aligned_alloc(alignment, size);

    size_t usable = malloc_usable_size(ptr);
    if (!((uintptr_t)ptr & (alignment - 1)))
    {
        if (usable >= size)
            return ptr;
    }

    void* newptr = std::aligned_alloc(alignment, size);
    if (!newptr)
        return nullptr;

    memcpy(newptr, ptr, usable < size ? usable : size);
    std::free(ptr);

    return newptr;
}

static AM_INLINE(size_t) am_aligned_msize(void* ptr, size_t alignment)
{
    if (alignment < sizeof(void*))
        alignment = sizeof(void*);

    return malloc_usable_size(ptr) - alignment - sizeof(void*);
}

#define AM_SIMD_ALIGNMENT 16
#define AM_TYPE_ALIGN_SIMD(_declaration_) AM_TYPE_ALIGN(_declaration_, AM_SIMD_ALIGNMENT)
#define AM_BUFFER_ALIGNMENT AM_SIMD_ALIGNMENT
#define AM_ALIGNED_ALLOC(_size_, _alignment_) std::aligned_alloc(_alignment_, _size_)
#define AM_ALIGNED_REALLOC(_ptr_, _size_, _alignment_) am_aligned_realloc(_ptr_, _size_, _alignment_)
#define AM_ALIGNED_MSIZE(_ptr_, _alignment_) am_aligned_msize(_ptr_, _alignment_)
#define AM_ALIGNED_FREE(_ptr_) std::free(_ptr_)

#if defined(AM_WCHAR_SUPPORTED)
#include <sstream>

// Defines the format used to print AmOsString text
#define AM_OS_CHAR_FMT "%ls"

// Macro used to convert a string literal to an AmOsString string at compile-time
#define AM_OS_STRING(s) L##s

AM_INLINE std::wstring am_string_widen(const std::string& str)
{
    std::wostringstream wstm;
    const auto& ctfacet = std::use_facet<std::ctype<wchar_t>>(wstm.getloc());
    for (char i : str)
        wstm << ctfacet.widen(i);
    return wstm.str();
}

AM_INLINE std::string am_wstring_narrow(const std::wstring& str)
{
    std::ostringstream stm;
    const auto& ctfacet = std::use_facet<std::ctype<wchar_t>>(stm.getloc());
    for (wchar_t i : str)
        stm << ctfacet.narrow(i, 0);
    return stm.str();
}

// Conversion between OS strings and default strings
#define AM_OS_STRING_TO_STRING(s) am_wstring_narrow(s).c_str()
#define AM_STRING_TO_OS_STRING(s) am_string_widen(s).c_str()
#else
// Defines the format used to print AmOsString text
#define AM_OS_CHAR_FMT "%s"

// Macro used to convert a string literal to an AmOsString string at compile-time
#define AM_OS_STRING(s) s

// Conversion between OS strings and default strings
#define AM_OS_STRING_TO_STRING(s) s
#define AM_STRING_TO_OS_STRING(s) s
#endif

// AMPLITUDE_ASSERT Config
#ifdef AMPLITUDE_NO_ASSERTS
#define AMPLITUDE_ASSERT(x)
#else
#include <cassert> // assert
#define AMPLITUDE_ASSERT(x) assert(x)
#endif

#endif // SS_AMPLITUDE_AUDIO_UNIX_CONFIG_H
