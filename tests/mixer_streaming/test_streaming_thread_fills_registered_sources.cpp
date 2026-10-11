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

#include <chrono>
#include <memory>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Mixer/Streaming/StreamingThread.h>

#include "ComponentTestCase.h"
#include "StreamTestDecoder.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    template<typename Predicate>
    static bool PollUntil(Predicate predicate, AmUInt32 timeoutMs = 2000)
    {
        for (AmUInt32 waited = 0; waited < timeoutMs; ++waited)
        {
            if (predicate())
                return true;
            Thread::Sleep(1);
        }

        return predicate();
    }

    AM_TEST_CASE(ComponentTestCase, mixer_streaming, streaming_thread_fills_registered_sources)
    {
    public:
        void Run() override
        {
            StreamingThread thread({ 100.0, 20.0, false, 256 });
            thread.Start();
            AM_EXPECT(thread.IsRunning());

            auto decoder = std::make_shared<StreamTestDecoder>(48000, 2);
            StreamPlan plan;
            plan.regionStart = 0;
            plan.regionEnd = 48000;
            plan.loop = false;

            StreamSource* source = thread.Create(decoder, plan, 2, 48000);
            AM_EXPECT(source != nullptr);

            AM_EXPECT(PollUntil(
                [&]()
                {
                    return thread.IsReady(source);
                }));
            AM_EXPECT(PollUntil(
                [&]()
                {
                    return source->GetRing().GetWritableFrames() == 0;
                }));
            AM_EXPECT_EQ(4800u, source->GetRing().GetCapacity());

            const auto threads = decoder->GetCallingThreads();
            AM_EXPECT_EQ(1u, threads.size());
            AM_EXPECT(threads.count(Thread::GetCurrentThreadId()) == 0);

            // The consumer frees space: the worker refills it.
            source->GetRing().Release(2400);
            AM_EXPECT(PollUntil(
                [&]()
                {
                    return source->GetRing().GetWritableFrames() == 0;
                }));
            AM_EXPECT_EQ(4800u, source->GetBufferedFrames());

            thread.Retire(source);
            AM_EXPECT(PollUntil(
                [&]()
                {
                    thread.CollectRetired();
                    return decoder->IsClosed();
                }));

            thread.Stop();
            AM_EXPECT(!thread.IsRunning());
        }
    };

    AM_REGISTER_TEST(mixer_streaming, streaming_thread_fills_registered_sources);
} // namespace SparkyStudios::Audio::Amplitude::Tests
