// Copyright (c) 2024-present Sparky Studios. All rights reserved.
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

#include <cstdarg>
#include <cstdio>

/**
 * @brief The log function, used in verbose mode.
 *
 * @param output The output stream.
 * @param fmt The message format.
 * @param ... The arguments.
 */
static void log(FILE* output, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
#if defined(AM_WCHAR_SUPPORTED)
    vfwprintf(output, AM_STRING_TO_OS_STRING(fmt), args);
#else
    vfprintf(output, fmt, args);
#endif
    va_end(args);
}
