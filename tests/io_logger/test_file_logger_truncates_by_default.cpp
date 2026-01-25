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

#include "SimpleTestCase.h"
#include "TestRegistry.h"

#include <filesystem>
#include <fstream>
#include <sstream>

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(SimpleTestCase, io_logger, file_logger_truncates_by_default)
    {
    public:
        void Run() override
        {
            // Save current logger
            Logger* prevLogger = Logger::GetLogger();

            const AmOsString logPath = AM_OS_STRING("./samples/assets/test_data/logger_test_truncate.log");
            const std::filesystem::path fsPath(logPath);

            // First logger - truncate mode (default append=false)
            {
                FileLogger fileLogger(logPath, false);
                AM_EXPECT(fileLogger.IsValid());
                Logger::SetLogger(&fileLogger);
                amLogInfo("original_content");
                Logger::SetLogger(prevLogger);
            }

            // Second logger - also truncate mode (default append=false)
            {
                FileLogger fileLogger(logPath, false);
                AM_EXPECT(fileLogger.IsValid());
                Logger::SetLogger(&fileLogger);
                amLogInfo("new_content");
                Logger::SetLogger(prevLogger);
            }

            // Read file and verify content
            std::ifstream file(fsPath);
            AM_EXPECT(file.is_open());

            std::stringstream buffer;
            buffer << file.rdbuf();
            std::string content = buffer.str();
            file.close();

            // Verify original_content is NOT present (was truncated)
            AM_EXPECT(content.find("original_content") == std::string::npos);

            // Verify new_content IS present
            AM_EXPECT(content.find("new_content") != std::string::npos);

            // Cleanup - delete test file
            std::filesystem::remove(fsPath);
        }
    };

    AM_REGISTER_TEST_DESKTOP_ONLY(io_logger, file_logger_truncates_by_default);
} // namespace SparkyStudios::Audio::Amplitude::Tests
