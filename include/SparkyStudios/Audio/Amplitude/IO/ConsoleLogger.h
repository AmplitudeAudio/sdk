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

#ifndef _AM_IO_CONSOLE_LOGGER_H
#define _AM_IO_CONSOLE_LOGGER_H

#include <SparkyStudios/Audio/Amplitude/IO/Log.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief The console logger class.
     *
     * This class logs messages to the console or terminal.
     *
     * @ingroup io
     */
    class AM_API_PUBLIC ConsoleLogger final : public Logger
    {
    public:
        /**
         * @brief Constructs a new console logger.
         *
         * @param[in] displayFileAndLine Whether to display the file and line number in the log messages.
         */
        explicit ConsoleLogger(bool displayFileAndLine = true);

        /**
         * @brief Destructor.
         */
        ~ConsoleLogger() override = default;

    protected:
        /**
         * @inherit
         */
        void Log(eLogMessageLevel level, const char* file, int line, const AmString& message) override;

    private:
        bool m_displayFileAndLine = true;
    };
} // namespace SparkyStudios::Audio::Amplitude
#endif // _AM_IO_CONSOLE_LOGGER_H
