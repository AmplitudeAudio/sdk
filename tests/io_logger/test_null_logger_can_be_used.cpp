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

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(SimpleTestCase, io_logger, null_logger_can_be_used)
    {
    public:
        void Run() override
        {
            // Save current logger
            Logger* prevLogger = Logger::GetLogger();

            // Create and set NullLogger
            NullLogger nullLogger;
            Logger::SetLogger(&nullLogger);

            // Verify NullLogger is now the global logger
            AM_EXPECT(amLogger == &nullLogger);

            // Call all log level methods - should not crash
            amLogDebug("Debug message from NullLogger test");
            amLogInfo("Info message from NullLogger test");
            amLogWarning("Warning message from NullLogger test");
            amLogError("Error message from NullLogger test");
            amLogCritical("Critical message from NullLogger test");
            amLogSuccess("Success message from NullLogger test");

            // Restore previous logger
            Logger::SetLogger(prevLogger);

            // Test passes if we reach here without crashing
            AM_EXPECT(true);
        }
    };

    AM_REGISTER_TEST(io_logger, null_logger_can_be_used);
} // namespace SparkyStudios::Audio::Amplitude::Tests
