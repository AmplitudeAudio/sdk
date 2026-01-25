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

#include <SparkyStudios/Audio/Amplitude/IO/FileLogger.h>

#include <chrono>
#include <filesystem>
#include <iomanip>
#include <sstream>

namespace SparkyStudios::Audio::Amplitude
{
    FileLogger::FileLogger(const AmOsString& filePath, bool append, bool displayFileAndLine)
        : m_displayFileAndLine(displayFileAndLine)
        , m_isValid(false)
    {
        std::ios_base::openmode mode = std::ios::out;
        if (append)
            mode |= std::ios::app;
        else
            mode |= std::ios::trunc;

        m_file.open(std::filesystem::path(filePath), mode);
        m_isValid = m_file.is_open();
    }

    FileLogger::~FileLogger()
    {
        if (m_file.is_open())
            m_file.close();
    }

    bool FileLogger::IsValid() const
    {
        return m_isValid;
    }

    void FileLogger::Log(eLogMessageLevel level, const char* file, int line, const AmString& message)
    {
        if (!m_isValid)
            return;

        std::lock_guard<std::mutex> lock(m_mutex);

        // Generate timestamp [YYYY-MM-DD HH:MM:SS.mmm]
        const auto now = std::chrono::system_clock::now();
        const auto timeT = std::chrono::system_clock::to_time_t(now);
        const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

        std::tm localTime{};
#if AM_PLATFORM_WIN
        localtime_s(&localTime, &timeT);
#else
        localtime_r(&timeT, &localTime);
#endif

        std::ostringstream timestamp;
        timestamp << "[" << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S") << "." << std::setfill('0') << std::setw(3) << ms.count() << "] ";

        // Write level tag
        switch (level)
        {
        case eLogMessageLevel_Debug:
            m_file << "[DEBUG] ";
            break;
        case eLogMessageLevel_Info:
            m_file << "[INFO] ";
            break;
        case eLogMessageLevel_Warning:
            m_file << "[WARNING] ";
            break;
        case eLogMessageLevel_Error:
            m_file << "[ERROR] ";
            break;
        case eLogMessageLevel_Critical:
            m_file << "[CRITICAL] ";
            break;
        case eLogMessageLevel_Success:
            m_file << "[SUCCESS] ";
            break;
        }

        // Write timestamp
        m_file << timestamp.str();

        // Write file and line if enabled
        if (m_displayFileAndLine)
            m_file << "(" << file << ":" << line << ") ";

        // Write message
        m_file << message;

        // Add newline if message doesn't end with one
        if (message.empty() || message.back() != '\n')
            m_file << std::endl;

        // Flush for crash safety
        m_file.flush();
    }
} // namespace SparkyStudios::Audio::Amplitude
