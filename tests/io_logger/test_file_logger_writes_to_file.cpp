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
    AM_TEST_CASE(ComponentTestCase, io_logger, file_logger_writes_to_file)
    {
    public:
        void Run() override
        {
            // Save current logger
            Logger* prevLogger = Logger::GetLogger();

            const AmOsString logPath = AM_OS_STRING("./samples/assets/test_data/logger_test_write.log");
            const std::filesystem::path fsPath(logPath);

            {
                // Create FileLogger
                FileLogger fileLogger(logPath);
                AM_EXPECT(fileLogger.IsValid());

                // Set as global logger
                Logger::SetLogger(&fileLogger);

                // Log messages at different levels
                amLogInfo("Test info message");
                amLogWarning("Test warning message");
                amLogError("Test error message");

                // Read file BEFORE destroying logger to verify flush-on-write (AC 13)
                std::ifstream preDestroyFile(fsPath);
                if (preDestroyFile.is_open())
                {
                    std::stringstream preBuffer;
                    preBuffer << preDestroyFile.rdbuf();
                    std::string preContent = preBuffer.str();
                    preDestroyFile.close();

                    // Verify content is already present (flush happened immediately)
                    AM_EXPECT(preContent.find("Test info message") != std::string::npos);
                }

                // Restore logger before FileLogger goes out of scope
                Logger::SetLogger(prevLogger);
            }

            // Read file after logger destruction
            std::ifstream file(fsPath);
            AM_EXPECT(file.is_open());

            std::stringstream buffer;
            buffer << file.rdbuf();
            std::string content = buffer.str();
            file.close();

            // Verify timestamp format [YYYY-MM-DD present
            AM_EXPECT(content.find("[20") != std::string::npos);

            // Verify level tags present
            AM_EXPECT(content.find("[INFO]") != std::string::npos);
            AM_EXPECT(content.find("[WARNING]") != std::string::npos);
            AM_EXPECT(content.find("[ERROR]") != std::string::npos);

            // Verify messages present
            AM_EXPECT(content.find("Test info message") != std::string::npos);
            AM_EXPECT(content.find("Test warning message") != std::string::npos);
            AM_EXPECT(content.find("Test error message") != std::string::npos);

            // Cleanup - delete test file
            std::filesystem::remove(fsPath);
        }
    };

    AM_REGISTER_TEST_DESKTOP_ONLY(io_logger, file_logger_writes_to_file);
} // namespace SparkyStudios::Audio::Amplitude::Tests
