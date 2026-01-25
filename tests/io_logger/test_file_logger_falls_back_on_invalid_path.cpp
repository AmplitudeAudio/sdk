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
    AM_TEST_CASE(SimpleTestCase, io_logger, file_logger_falls_back_on_invalid_path)
    {
    public:
        void Run() override
        {
            // Save current logger
            Logger* prevLogger = Logger::GetLogger();

            // Create FileLogger with invalid path
            const AmOsString invalidPath = AM_OS_STRING("/nonexistent/path/that/should/not/exist/file.log");
            FileLogger fileLogger(invalidPath);

            // Verify IsValid() returns false
            AM_EXPECT(!fileLogger.IsValid());

            // Set as global logger anyway
            Logger::SetLogger(&fileLogger);

            // Call log methods - should not crash (no-op behavior)
            amLogInfo("This message should be silently discarded");
            amLogWarning("This warning should be silently discarded");
            amLogError("This error should be silently discarded");

            // Restore previous logger
            Logger::SetLogger(prevLogger);

            // Test passes if we reach here without crashing
            AM_EXPECT(true);
        }
    };

    AM_REGISTER_TEST_DESKTOP_ONLY(io_logger, file_logger_falls_back_on_invalid_path);
} // namespace SparkyStudios::Audio::Amplitude::Tests
