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

#include <memory>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Mixer/Streaming/StreamingThread.h>

#include "ComponentTestCase.h"
#include "StreamTestDecoder.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    namespace
    {
        template<typename Predicate>
        bool PollUntil(Predicate predicate, AmUInt32 timeoutMs = 2000)
        {
            for (AmUInt32 waited = 0; waited < timeoutMs; ++waited)
            {
                if (predicate())
                    return true;
                Thread::Sleep(1);
            }

            return predicate();
        }
    } // namespace

    // The readers drain the rings without signalling, so the worker polls while it has sources. Without any, it sleeps
    // until a source is created or retired: an engine that never streams does not wake it up every few milliseconds.
    AM_TEST_CASE(ComponentTestCase, mixer_streaming, streaming_thread_sleeps_without_sources)
    {
    public:
        void Run() override
        {
            StreamingThread thread({ 100.0, 20.0, false, 256 });
            thread.Start();
            AM_EXPECT(thread.IsRunning());

            // Idle: the first pass, and nothing after it (polling would have made about 20).
            Thread::Sleep(100);
            const AmUInt64 idle = thread.GetPassCount();
            AM_EXPECT(idle <= 2u);

            // A new source wakes it up at once.
            auto decoder = std::make_shared<StreamTestDecoder>(48000, 2);
            StreamPlan plan;
            plan.regionStart = 0;
            plan.regionEnd = 48000;

            StreamSource* source = thread.Create(decoder, plan, 2, 48000);
            AM_EXPECT(PollUntil(
                [&]()
                {
                    return source->GetRing().GetWritableFrames() == 0;
                }));
            AM_EXPECT(thread.GetPassCount() > idle);

            // Retiring the last source is acknowledged, then the worker sleeps again.
            thread.Retire(source);
            AM_EXPECT(PollUntil(
                [&]()
                {
                    thread.CollectRetired();
                    return decoder->IsClosed();
                }));

            Thread::Sleep(20);
            const AmUInt64 retired = thread.GetPassCount();
            Thread::Sleep(100);
            AM_EXPECT(thread.GetPassCount() - retired <= 1u);

            // Stop still wakes it.
            thread.Stop();
            AM_EXPECT(!thread.IsRunning());
        }
    };

    AM_REGISTER_TEST(mixer_streaming, streaming_thread_sleeps_without_sources);
} // namespace SparkyStudios::Audio::Amplitude::Tests
