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

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(ComponentTestCase, io_logger, console_logger_can_be_created)
    {
    public:
        void Run() override
        {
            // Save current logger
            Logger* prevLogger = Logger::GetLogger();

            // Test default constructor (displayFileAndLine = true)
            {
                ConsoleLogger consoleLogger;
                Logger::SetLogger(&consoleLogger);

                // Verify ConsoleLogger is now the global logger
                AM_EXPECT(amLogger == &consoleLogger);

                // Log a message to verify default config works
                amLogInfo("ConsoleLogger default constructor test");
            }

            // Test with displayFileAndLine = false
            {
                ConsoleLogger consoleLoggerNoFileLine(false);
                Logger::SetLogger(&consoleLoggerNoFileLine);

                // Verify it's set
                AM_EXPECT(amLogger == &consoleLoggerNoFileLine);

                // Log a message to verify it works
                amLogInfo("ConsoleLogger with displayFileAndLine=false");
            }

            // Restore previous logger
            Logger::SetLogger(prevLogger);

            // Test passes if we reach here without crashing
            AM_EXPECT(true);
        }
    };

    AM_REGISTER_TEST(io_logger, console_logger_can_be_created);
} // namespace SparkyStudios::Audio::Amplitude::Tests
