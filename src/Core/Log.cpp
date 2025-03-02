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

#if AM_PLATFORM_ANDROID
#include <Core/Platforms/Android/AndroidConsoleLogger.h>
#else
#include <iostream>
#endif

#include <SparkyStudios/Audio/Amplitude/Core/Log.h>

namespace SparkyStudios::Audio::Amplitude
{
    static Logger* gLogger = nullptr;

    static auto& getLogger(eLogMessageLevel level)
    {
#if AM_PLATFORM_ANDROID
        switch (level)
        {
        case eLogMessageLevel_Debug:
            return aout_d;
        case eLogMessageLevel_Info:
            return aout_i;
        case eLogMessageLevel_Warning:
            return aout_w;
        case eLogMessageLevel_Error:
            return aout_e;
        case eLogMessageLevel_Critical:
            return aout_f;
        default:
        case eLogMessageLevel_Success:
            return aout_s;
        }
#else
        return std::cout;
#endif
    }

    void Logger::SetLogger(Logger* loggerInstance)
    {
        gLogger = loggerInstance;
    }

    Logger* Logger::GetLogger()
    {
        return gLogger;
    }

    void Logger::Debug(const char* file, int line, const AmString& message)
    {
#ifdef AM_DEBUG
        Log(eLogMessageLevel_Debug, file, line, message);
#endif
    }

    void Logger::Info(const char* file, int line, const AmString& message)
    {
        Log(eLogMessageLevel_Info, file, line, message);
    }

    void Logger::Warning(const char* file, int line, const AmString& message)
    {
        Log(eLogMessageLevel_Warning, file, line, message);
    }

    void Logger::Error(const char* file, int line, const AmString& message)
    {
        Log(eLogMessageLevel_Error, file, line, message);
    }

    void Logger::Critical(const char* file, int line, const AmString& message)
    {
        Log(eLogMessageLevel_Critical, file, line, message);
    }

    void Logger::Success(const char* file, int line, const AmString& message)
    {
        Log(eLogMessageLevel_Success, file, line, message);
    }

    ConsoleLogger::ConsoleLogger(bool displayFileAndLine)
        : m_displayFileAndLine(displayFileAndLine)
    {}

    void ConsoleLogger::Log(eLogMessageLevel level, const char* file, int line, const AmString& message)
    {
        auto& out = getLogger(level);

        switch (level)
        {
        case eLogMessageLevel_Debug:
            out << "[DEBUG] ";
            break;
        case eLogMessageLevel_Info:
            out << "[INFO] ";
            break;
        case eLogMessageLevel_Warning:
            out << "[WARNING] ";
            break;
        case eLogMessageLevel_Error:
            out << "[ERROR] ";
            break;
        case eLogMessageLevel_Critical:
            out << "[CRITICAL] ";
            break;
        case eLogMessageLevel_Success:
            out << "[SUCCESS] ";
            break;
        }

        if (m_displayFileAndLine)
            out << "(" << file << ":" << line << ") ";

        AmString m = message;
        if (message.back() == '\n')
            m.pop_back();

        out << m << std::endl;
    }
} // namespace SparkyStudios::Audio::Amplitude
