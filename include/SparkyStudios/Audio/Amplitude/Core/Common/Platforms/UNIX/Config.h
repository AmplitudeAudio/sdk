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
#define AM_SIMD_ALIGNMENT 16
#define AM_ALIGN_SIMD(_declaration_) AM_ALIGN(_declaration_, AM_SIMD_ALIGNMENT)
#define AM_BUFFER_ALIGNMENT AM_SIMD_ALIGNMENT

// Defines the format used to print AmOsString text
#define AM_OS_CHAR_FMT "%s"

// Macro used to convert a string literal to an AmOsString string at compile-time
#define AM_OS_STRING(s) s

// Conversion between OS strings and default strings
#define AM_OS_STRING_TO_STRING(s) s
#define AM_STRING_TO_OS_STRING(s) s

// AMPLITUDE_ASSERT Config
#ifdef AMPLITUDE_NO_ASSERTS
#define AMPLITUDE_ASSERT(x)
#else
#include <cassert> // assert
#define AMPLITUDE_ASSERT(x) assert(x)
#endif

#endif // SS_AMPLITUDE_AUDIO_UNIX_CONFIG_H
