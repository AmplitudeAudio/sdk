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
    // Outside an audio thread nothing is deferred: the message reaches the logger before the call returns.
    AM_TEST_CASE(PureUnitTestCase, io_logger, logger_writes_at_once_off_the_audio_thread)
    {
    public:
        void Run() override
        {
            CaptureLogger capture;
            ScopedGlobalLogger scope(&capture);

            amLogInfo("written %d", 1);

            const auto entries = capture.Entries();
            AM_EXPECT_EQ(1u, entries.size());
            if (entries.size() == 1)
            {
                AM_EXPECT(entries[0].first == eLogMessageLevel_Info);
                AM_EXPECT(entries[0].second == "written 1");
            }
        }
    };

    AM_REGISTER_TEST(io_logger, logger_writes_at_once_off_the_audio_thread);
} // namespace SparkyStudios::Audio::Amplitude::Tests
