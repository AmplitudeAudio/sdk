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

#include <string>
#include <thread>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include "CaptureLogger.h"
#include "PureUnitTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    // Replacing the logger writes what the audio thread queued for it, so nothing is lost with the old logger.
    AM_TEST_CASE(PureUnitTestCase, io_logger, logger_set_logger_flushes_the_previous_one)
    {
    public:
        void Run() override
        {
            CaptureLogger first;
            CaptureLogger second;
            ScopedGlobalLogger scope(&first);

            {
                const Logger::ScopedAudioThread audioThread;
                amLogInfo("for the first logger");
            }

            AM_EXPECT_EQ(0u, first.Count());

            Logger::SetLogger(&second);

            AM_EXPECT_EQ(1u, first.Count());
            AM_EXPECT_EQ(0u, second.Count());

            Logger::SetLogger(&first);
        }
    };

    AM_REGISTER_TEST(io_logger, logger_set_logger_flushes_the_previous_one);
} // namespace SparkyStudios::Audio::Amplitude::Tests
