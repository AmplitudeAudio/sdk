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

#include <atomic>
#include <chrono>
#include <future>
#include <memory>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Mixer/Streaming/StreamingThread.h>

#include "ComponentTestCase.h"
#include "SlowCountingDecoder.h"
#include "StreamTestDecoder.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(ComponentTestCase, mixer_streaming, streaming_thread_retire_racing_stop_closes_once)
    {
    public:
        void Run() override
        {
            std::atomic<int> closeCount{ 0 };
            std::atomic<bool> stopReturned{ false };
            std::atomic<bool> closedBeforeStop{ false };

            StreamingThread thread({ 100.0, 20.0, false, 256 });
            thread.Start();

            auto decoder = std::make_shared<SlowCountingDecoder>(closeCount, stopReturned, closedBeforeStop);
            StreamPlan plan;
            plan.regionStart = 0;
            plan.regionEnd = 48000 * 10;

            // The worker is already running here, so the first fill happens on the worker, inside the slow Stream() call.
            StreamSource* source = thread.Create(decoder, plan, 2, 48000);
            // Without the worker inside the slow read, the race below is not exercised and the test would pass
            // vacuously: fail instead.
            const bool inStream = decoder->WaitUntilInStream();
            AM_EXPECT(inStream);
            if (!inStream)
            {
                thread.Stop();
                return;
            }

            auto stopped = std::async(
                std::launch::async,
                [&]()
                {
                    thread.Stop();
                });
            Thread::Sleep(20); // Stop() is now joining a worker stuck in the slow Stream() call

            // Retiring while Stop() joins must queue the source, not free it under the worker.
            thread.Retire(source);

            AM_EXPECT(stopped.wait_for(std::chrono::seconds(2)) == std::future_status::ready);
            stopReturned.store(true);

            // The decoder must not close while the worker could still use it.
            AM_EXPECT(!closedBeforeStop.load());
            AM_EXPECT_EQ(0, closeCount.load());

            thread.CollectRetired();
            AM_EXPECT_EQ(1, closeCount.load());

            thread.CollectRetired();
            AM_EXPECT_EQ(1, closeCount.load());
        }
    };

    AM_REGISTER_TEST(mixer_streaming, streaming_thread_retire_racing_stop_closes_once);
} // namespace SparkyStudios::Audio::Amplitude::Tests
