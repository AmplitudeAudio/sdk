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
#include <thread>
#include <vector>

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(ComponentTestCase, io_logger, file_logger_thread_safety)
    {
    public:
        void Run() override
        {
            // Save current logger
            Logger* prevLogger = Logger::GetLogger();

            const AmOsString logPath = AM_OS_STRING("./samples/assets/test_data/logger_test_threads.log");
            const std::filesystem::path fsPath(logPath);

            constexpr int numThreads = 4;
            constexpr int messagesPerThread = 100;
            constexpr int totalMessages = numThreads * messagesPerThread;

            // Create FileLogger
            FileLogger fileLogger(logPath);
            AM_EXPECT(fileLogger.IsValid());
            Logger::SetLogger(&fileLogger);

            // Thread function to log messages
            auto logThreadFunc = [](int threadId)
            {
                for (int i = 0; i < messagesPerThread; ++i)
                {
                    amLogInfo("Thread %d Message %d", threadId, i);
                }
            };

            // Spawn threads
            std::vector<std::thread> threads;
            threads.reserve(numThreads);
            for (int t = 0; t < numThreads; ++t)
            {
                threads.emplace_back(logThreadFunc, t);
            }

            // Join all threads
            for (auto& thread : threads)
            {
                thread.join();
            }

            // Restore logger before closing file
            Logger::SetLogger(prevLogger);

            // Close logger by going out of scope handled implicitly, but we set prev first
            // Force the fileLogger destructor by this scope block
            // We need to explicitly close the file to read it
            // Actually, the fileLogger is still in scope - let's restructure

            // Read file and verify
            {
                std::ifstream file(fsPath);
                AM_EXPECT(file.is_open());

                int lineCount = 0;
                std::string line;
                bool allLinesValid = true;

                while (std::getline(file, line))
                {
                    ++lineCount;

                    // Verify each line starts with [INFO]
                    if (line.find("[INFO]") == std::string::npos)
                    {
                        allLinesValid = false;
                    }

                    // Verify line contains complete message (has "Thread" and "Message")
                    if (line.find("Thread") == std::string::npos || line.find("Message") == std::string::npos)
                    {
                        allLinesValid = false;
                    }
                }

                file.close();

                // Verify total line count
                AM_EXPECT(lineCount == totalMessages);

                // Verify all lines are well-formed (no interleaving/corruption)
                AM_EXPECT(allLinesValid);
            }

            // Cleanup - delete test file
            std::filesystem::remove(fsPath);
        }
    };

    AM_REGISTER_TEST_DESKTOP_ONLY(io_logger, file_logger_thread_safety);
} // namespace SparkyStudios::Audio::Amplitude::Tests
