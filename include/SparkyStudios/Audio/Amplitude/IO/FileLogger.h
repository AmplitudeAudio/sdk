// Copyright (c) 2026-present Sparky Studios. All rights reserved.
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

#ifndef _AM_IO_FILE_LOGGER_H
#define _AM_IO_FILE_LOGGER_H

#include <SparkyStudios/Audio/Amplitude/IO/Log.h>

#include <fstream>
#include <mutex>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief A @ref Logger implementation that writes log messages to a file.
     *
     * This logger writes timestamped log messages to a specified file path.
     * It supports both append and truncate modes, and is thread-safe.
     *
     * If the file cannot be opened, the logger degrades gracefully to a no-op
     * (similar to NullLogger). Use @ref IsValid() to check if the file was
     * opened successfully.
     *
     * @ingroup io
     */
    class AM_API_PUBLIC FileLogger final : public Logger
    {
    public:
        /**
         * @brief Constructs a new file logger.
         *
         * @param[in] filePath The path to the log file.
         * @param[in] append If true, append to existing file; if false (default), truncate.
         * @param[in] displayFileAndLine Whether to display the file and line number in log messages.
         */
        explicit FileLogger(const AmOsString& filePath, bool append = false, bool displayFileAndLine = true);

        /**
         * @brief Destructor that closes the file stream.
         */
        ~FileLogger() override;

        /**
         * @brief Checks if the log file was opened successfully.
         *
         * @return true if the file is open and ready for writing, false otherwise.
         */
        [[nodiscard]] bool IsValid() const;

    protected:
        /**
         * @inherit
         */
        void Log(eLogMessageLevel level, const char* file, int line, const AmString& message) override;

    private:
        std::ofstream m_file;
        std::mutex m_mutex;
        bool m_displayFileAndLine;
        bool m_isValid;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IO_FILE_LOGGER_H
