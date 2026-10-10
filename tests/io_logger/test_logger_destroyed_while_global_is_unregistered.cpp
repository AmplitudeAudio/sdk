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

#include "CaptureLogger.h"
#include "PureUnitTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    // A logger destroyed while it is the global one stops being it: the next SetLogger() and the log macros must not
    // reach a dead object.
    AM_TEST_CASE(PureUnitTestCase, io_logger, logger_destroyed_while_global_is_unregistered)
    {
    public:
        void Run() override
        {
            Logger* previous = Logger::GetLogger();

            {
                CaptureLogger capture;
                Logger::SetLogger(&capture);
                AM_EXPECT(amLogger == &capture);
            }

            AM_EXPECT(amLogger == nullptr);

            // Replacing it is safe, and so is logging with no logger at all.
            CaptureLogger next;
            Logger::SetLogger(&next);
            AM_EXPECT(amLogger == &next);

            Logger::SetLogger(nullptr);
            amLogInfo("goes nowhere");
            AM_EXPECT_EQ(0u, next.Count());

            Logger::SetLogger(previous);
        }
    };

    AM_REGISTER_TEST(io_logger, logger_destroyed_while_global_is_unregistered);
} // namespace SparkyStudios::Audio::Amplitude::Tests
