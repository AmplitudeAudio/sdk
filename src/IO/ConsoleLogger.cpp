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

#include <iostream>

#include <SparkyStudios/Audio/Amplitude/IO/ConsoleLogger.h>

namespace SparkyStudios::Audio::Amplitude
{
    ConsoleLogger::ConsoleLogger(bool displayFileAndLine)
        : m_displayFileAndLine(displayFileAndLine)
    {}

    void ConsoleLogger::Log(eLogMessageLevel level, const char* file, int line, const AmString& message)
    {
        switch (level)
        {
        case eLogMessageLevel_Debug:
            std::cout << "[DEBUG] ";
            break;
        case eLogMessageLevel_Info:
            std::cout << "[INFO] ";
            break;
        case eLogMessageLevel_Warning:
            std::cout << "[WARNING] ";
            break;
        case eLogMessageLevel_Error:
            std::cout << "[ERROR] ";
            break;
        case eLogMessageLevel_Critical:
            std::cout << "[CRITICAL] ";
            break;
        case eLogMessageLevel_Success:
            std::cout << "[SUCCESS] ";
            break;
        }

        if (m_displayFileAndLine)
            std::cout << "(" << file << ":" << line << ") ";

        std::cout << message;

        if (message.back() != '\n')
            std::cout << std::endl;
    }
} // namespace SparkyStudios::Audio::Amplitude