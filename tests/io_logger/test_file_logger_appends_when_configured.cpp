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

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include "ComponentTestCase.h"
#include "TestRegistry.h"

#include <filesystem>
#include <fstream>
#include <sstream>

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(ComponentTestCase, io_logger, file_logger_appends_when_configured)
    {
    public:
        void Run() override
        {
            // Save current logger
            Logger* prevLogger = Logger::GetLogger();

            const AmOsString logPath = AM_OS_STRING("./samples/assets/test_data/logger_test_append.log");
            const std::filesystem::path fsPath(logPath);

            // First logger - truncate mode (append=false)
            {
                FileLogger fileLogger(logPath, false);
                AM_EXPECT(fileLogger.IsValid());
                Logger::SetLogger(&fileLogger);
                amLogInfo("first_message");
                Logger::SetLogger(prevLogger);
            }

            // Second logger - append mode (append=true)
            {
                FileLogger fileLogger(logPath, true);
                AM_EXPECT(fileLogger.IsValid());
                Logger::SetLogger(&fileLogger);
                amLogInfo("second_message");
                Logger::SetLogger(prevLogger);
            }

            // Read file and verify both messages present
            std::ifstream file(fsPath);
            AM_EXPECT(file.is_open());

            std::stringstream buffer;
            buffer << file.rdbuf();
            std::string content = buffer.str();
            file.close();

            // Verify both messages are present (append preserved first message)
            AM_EXPECT(content.find("first_message") != std::string::npos);
            AM_EXPECT(content.find("second_message") != std::string::npos);

            // Cleanup - delete test file
            std::filesystem::remove(fsPath);
        }
    };

    AM_REGISTER_TEST_DESKTOP_ONLY(io_logger, file_logger_appends_when_configured);
} // namespace SparkyStudios::Audio::Amplitude::Tests
