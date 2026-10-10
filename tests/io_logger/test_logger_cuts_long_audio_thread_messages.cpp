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
    // The queue keeps fixed-size entries: a longer message logged from an audio thread is cut, not dropped.
    AM_TEST_CASE(PureUnitTestCase, io_logger, logger_cuts_long_audio_thread_messages)
    {
    public:
        void Run() override
        {
            CaptureLogger capture;
            ScopedGlobalLogger scope(&capture);

            {
                const Logger::ScopedAudioThread audioThread;
                const std::string longMessage(Logger::kMaxQueuedMessageLength + 100, 'x');
                amLogInfo("%s", longMessage.c_str());
                amLogInfo("short");
            }

            amLogger->Flush();

            const auto entries = capture.Entries();
            AM_EXPECT_EQ(2u, entries.size());
            if (entries.size() == 2)
            {
                AM_EXPECT_EQ(static_cast<AmSize>(Logger::kMaxQueuedMessageLength), entries[0].second.size());
                AM_EXPECT(entries[1].second == "short");
            }
        }
    };

    AM_REGISTER_TEST(io_logger, logger_cuts_long_audio_thread_messages);
} // namespace SparkyStudios::Audio::Amplitude::Tests
