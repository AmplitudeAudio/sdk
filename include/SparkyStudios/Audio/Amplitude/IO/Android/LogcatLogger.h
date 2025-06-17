// Copyright (c) 2025-present Sparky Studios. All rights reserved.
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

#if AM_PLATFORM_ANDROID

#ifndef _AM_IO_ANDROID_LOGCAT_LOGGER_H
#define _AM_IO_ANDROID_LOGCAT_LOGGER_H

#include <SparkyStudios/Audio/Amplitude/IO/Log.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief A logger that writes to Android's logcat.
     *
     * This class logs output in the Android's logcat using a custom tag.
     *
     * @ingroup io
     */
    class AM_API_PUBLIC LogcatLogger final : public Logger
    {
    public:
        /**
         * @brief Constructs a new logcat logger.
         *
         * @param[in] logTag A custom tag to use when logging in logcat. Defaults to @b AM.
         */
        explicit LogcatLogger(const char* logTag = "AM");

        /**
         * @brief Default destructor.
         */
        ~LogcatLogger() override = default;

    protected:
        /**
         * @inherit
         */
        void Log(eLogMessageLevel level, const char* file, int line, const AmString& message) override;

    private:
        const char* m_logTag;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IO_ANDROID_LOGCAT_LOGGER_H

#endif // AM_PLATFORM_ANDROID