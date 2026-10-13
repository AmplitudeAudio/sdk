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
    // A full queue drops the extra messages, and the next flush says how many.
    AM_TEST_CASE(PureUnitTestCase, io_logger, logger_reports_dropped_audio_thread_messages)
    {
    public:
        void Run() override
        {
            constexpr int kLogged = 300;

            CaptureLogger capture;
            ScopedGlobalLogger scope(&capture);

            {
                const Logger::ScopedAudioThread audioThread;
                for (int i = 0; i < kLogged; ++i)
                    amLogInfo("message %d", i);
            }

            amLogger->Flush();

            const auto entries = capture.Entries();
            AM_EXPECT(entries.size() > 1 && entries.size() < static_cast<AmSize>(kLogged));
            if (entries.size() < 2)
                return;

            const AmSize kept = entries.size() - 1;

            // The first messages are the ones kept, in order, and the last entry is the report.
            AM_EXPECT(entries.front().second == "message 0");
            AM_EXPECT(entries[kept - 1].second == "message " + std::to_string(kept - 1));
            AM_EXPECT(entries.back().first == eLogMessageLevel_Warning);
            AM_EXPECT(entries.back().second.find(std::to_string(kLogged - kept) + " log message") == 0);

            // The count was reset: a second flush reports nothing more.
            amLogger->Flush();
            AM_EXPECT_EQ(entries.size(), capture.Count());
        }
    };

    AM_REGISTER_TEST(io_logger, logger_reports_dropped_audio_thread_messages);
} // namespace SparkyStudios::Audio::Amplitude::Tests
