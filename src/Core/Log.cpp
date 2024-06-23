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

#include <iostream>

#include <SparkyStudios/Audio/Amplitude/Core/Log.h>

namespace SparkyStudios::Audio::Amplitude
{
    static Logger* gLogger = nullptr;

    void Logger::SetLogger(Logger* loggerInstance)
    {
        gLogger = loggerInstance;
    }

    Logger* Logger::GetLogger()
    {
        return gLogger;
    }

    void Logger::Debug(const char* file, int line, const AmOsString& message)
    {
#ifdef AM_DEBUG
        Log(eLML_DEBUG, file, line, message);
#endif
    }

    void Logger::Info(const char* file, int line, const AmOsString& message)
    {
        Log(eLML_INFO, file, line, message);
    }

    void Logger::Warning(const char* file, int line, const AmOsString& message)
    {
        Log(eLML_WARNING, file, line, message);
    }

    void Logger::Error(const char* file, int line, const AmOsString& message)
    {
        Log(eLML_ERROR, file, line, message);
    }

    void Logger::Critical(const char* file, int line, const AmOsString& message)
    {
        Log(eLML_CRITICAL, file, line, message);
    }

    ConsoleLogger::ConsoleLogger(bool displayFileAndLine)
        : m_displayFileAndLine(displayFileAndLine)
    {}

    void ConsoleLogger::Log(LogMessageLevel level, const char* file, int line, const AmOsString& message)
    {
        switch (level)
        {
        case eLML_DEBUG:
            std::cout << "[DEBUG] ";
            break;
        case eLML_INFO:
            std::cout << "[INFO] ";
            break;
        case eLML_WARNING:
            std::cout << "[WARNING] ";
            break;
        case eLML_ERROR:
            std::cout << "[ERROR] ";
            break;
        case eLML_CRITICAL:
            std::cout << "[CRITICAL] ";
            break;
        }

        if (m_displayFileAndLine)
            std::cout << "(" << file << ":" << line << ") ";

#if defined(AM_WCHAR_SUPPORTED)
        std::wcout << message;
#else
        std::cout << message;
#endif

        if (message.back() != '\n')
            std::cout << std::endl;
    }
} // namespace SparkyStudios::Audio::Amplitude
