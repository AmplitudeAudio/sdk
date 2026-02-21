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
    AM_TEST_CASE(ComponentTestCase, io_logger, file_logger_display_file_line_toggle)
    {
    public:
        void Run() override
        {
            // Save current logger
            Logger* prevLogger = Logger::GetLogger();

            const AmOsString logPathOn = AM_OS_STRING("./samples/assets/test_data/logger_test_fileline_on.log");
            const AmOsString logPathOff = AM_OS_STRING("./samples/assets/test_data/logger_test_fileline_off.log");
            const std::filesystem::path fsPathOn(logPathOn);
            const std::filesystem::path fsPathOff(logPathOff);

            // Test with displayFileAndLine = true
            {
                FileLogger fileLogger(logPathOn, false, true);
                AM_EXPECT(fileLogger.IsValid());
                Logger::SetLogger(&fileLogger);
                amLogInfo("Test message with file and line");
                Logger::SetLogger(prevLogger);
            }

            // Read file and verify file:line present
            {
                std::ifstream file(fsPathOn);
                AM_EXPECT(file.is_open());

                std::stringstream buffer;
                buffer << file.rdbuf();
                std::string content = buffer.str();
                file.close();

                // Verify file:line pattern present (e.g., ".cpp:" or file name with colon)
                AM_EXPECT(content.find(".cpp:") != std::string::npos);
            }

            // Test with displayFileAndLine = false
            {
                FileLogger fileLogger(logPathOff, false, false);
                AM_EXPECT(fileLogger.IsValid());
                Logger::SetLogger(&fileLogger);
                amLogInfo("Test message without file and line");
                Logger::SetLogger(prevLogger);
            }

            // Read file and verify NO file:line pattern
            {
                std::ifstream file(fsPathOff);
                AM_EXPECT(file.is_open());

                std::stringstream buffer;
                buffer << file.rdbuf();
                std::string content = buffer.str();
                file.close();

                // Verify NO .cpp: pattern present
                AM_EXPECT(content.find(".cpp:") == std::string::npos);
            }

            // Cleanup - delete both test files
            std::filesystem::remove(fsPathOn);
            std::filesystem::remove(fsPathOff);
        }
    };

    AM_REGISTER_TEST_DESKTOP_ONLY(io_logger, file_logger_display_file_line_toggle);
} // namespace SparkyStudios::Audio::Amplitude::Tests
