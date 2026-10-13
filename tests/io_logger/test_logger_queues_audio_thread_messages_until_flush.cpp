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
    // A message logged inside a ScopedAudioThread is only queued; the next Flush() writes it, before any message logged
    // after it from another thread, and another thread is not affected by the scope of this one.
    AM_TEST_CASE(PureUnitTestCase, io_logger, logger_queues_audio_thread_messages_until_flush)
    {
    public:
        void Run() override
        {
            CaptureLogger capture;
            ScopedGlobalLogger scope(&capture);

            {
                const Logger::ScopedAudioThread audioThread;
                {
                    const Logger::ScopedAudioThread nested;
                }

                amLogWarning("queued %d", 1);
                amLogError("queued %d", 2);

                // Still on an audio thread after a nested scope ended, and nothing was written.
                AM_EXPECT_EQ(0u, capture.Count());

                // Another thread is not an audio thread: it writes at once, after the queued messages.
                std::thread other(
                    []()
                    {
                        amLogInfo("direct");
                    });
                other.join();

                const auto entries = capture.Entries();
                AM_EXPECT_EQ(3u, entries.size());
                if (entries.size() == 3)
                {
                    AM_EXPECT(entries[0].second == "queued 1");
                    AM_EXPECT(entries[0].first == eLogMessageLevel_Warning);
                    AM_EXPECT(entries[1].second == "queued 2");
                    AM_EXPECT(entries[1].first == eLogMessageLevel_Error);
                    AM_EXPECT(entries[2].second == "direct");
                }
            }

            // The scope ended: this thread writes at once again.
            amLogInfo("after");
            AM_EXPECT_EQ(4u, capture.Count());

            // A flush with nothing queued writes nothing.
            amLogger->Flush();
            AM_EXPECT_EQ(4u, capture.Count());
        }
    };

    AM_REGISTER_TEST(io_logger, logger_queues_audio_thread_messages_until_flush);
} // namespace SparkyStudios::Audio::Amplitude::Tests
