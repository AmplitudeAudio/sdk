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

#ifndef SPARK_AUDIO_LOG_H
#define SPARK_AUDIO_LOG_H

#include <cstdarg>

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief The function signature of the logging function.
     *
     * In order to perform logging, the library needs to be provided with a logging
     * function that fits this type signature.
     */
    typedef void (*LogFunc)(const char* fmt, va_list args);

    /**
     * @brief Register a logging function with the library.
     *
     * @param[in] The function to use for logging.
     */
    void RegisterLogFunc(LogFunc log_func);

    /**
     * @brief Call the registered log function with the provided format string.
     *
     * This does nothing if no logging function has been registered.
     *
     * @param[in] format The format string to print.
     * @param[in] ... The arguments to format.
     */
    void CallLogFunc(const char* format, ...);
} // namespace SparkyStudios::Audio::Amplitude

#endif // SPARK_AUDIO_LOG_H
