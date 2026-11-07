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

    AM_TEST_CASE(ComponentTestCase, mixer_streaming, streaming_thread_opens_deferred_decoders)
    {
    public:
        void Run() override
        {
            StreamPlan plan;
            plan.regionStart = 0;
            plan.regionEnd = 48000;

            // The worker opens the decoder: the creating thread never touches the file.
            {
                StreamingThread thread({ 100.0, 20.0, false, 256 });
                thread.Start();
                AM_EXPECT(thread.IsRunning());

                auto decoder = std::make_shared<StreamTestDecoder>(48000, 2);
                std::atomic<AmUInt64> openedOn{ 0 };
                StreamSource* source = thread.CreateDeferred(
                    [&]() -> std::shared_ptr<Codec::Decoder>
                    {
                        openedOn.store(Thread::GetCurrentThreadId());
                        return decoder;
                    },
                    plan, 2, 48000);
                thread.WakeUp();

                AM_EXPECT(PollUntil(
                    [&]()
                    {
                        return thread.IsReady(source);
                    }));
                AM_EXPECT(openedOn.load() != 0);
                AM_EXPECT(openedOn.load() != Thread::GetCurrentThreadId());
                AM_EXPECT(!source->GetRing().IsFailed());

                thread.Retire(source);
                thread.Stop();
                thread.CollectRetired();
                AM_EXPECT(decoder->IsClosed());
            }

            // A decoder that cannot be opened fails its stream once: it is ready, failed, and never retried.
            {
                StreamingThread thread({ 100.0, 20.0, true, 256 });

                AmUInt32 attempts = 0;
                StreamSource* source = thread.CreateDeferred(
                    [&]() -> std::shared_ptr<Codec::Decoder>
                    {
                        ++attempts;
                        return nullptr;
                    },
                    plan, 2, 48000);

                AM_EXPECT(thread.IsReady(source));
                AM_EXPECT(source->GetRing().IsFailed());

                thread.FillInline(source, 1000);
                AM_EXPECT_EQ(1u, attempts);

                thread.Retire(source);
            }
        }
    };

    AM_REGISTER_TEST(mixer_streaming, streaming_thread_opens_deferred_decoders);
} // namespace SparkyStudios::Audio::Amplitude::Tests
