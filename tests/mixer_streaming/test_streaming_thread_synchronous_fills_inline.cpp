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
    AM_TEST_CASE(ComponentTestCase, mixer_streaming, streaming_thread_synchronous_fills_inline)
    {
    public:
        void Run() override
        {
            StreamingThread thread({ 100.0, 20.0, true, 256 });
            AM_EXPECT(thread.IsSynchronous());

            // Start is a no-op in synchronous mode.
            thread.Start();
            AM_EXPECT(!thread.IsRunning());

            auto decoder = std::make_shared<StreamTestDecoder>(48000, 2);
            StreamPlan plan;
            plan.regionStart = 0;
            plan.regionEnd = 48000;

            StreamSource* source = thread.Create(decoder, plan, 2, 48000);
            AM_EXPECT(source != nullptr);

            AM_EXPECT_EQ(960u, thread.GetPrefillFrames(48000));
            AM_EXPECT(source->GetBufferedFrames() >= 960u);
            AM_EXPECT(thread.IsReady(source));

            const auto threads = decoder->GetCallingThreads();
            AM_EXPECT_EQ(1u, threads.size());
            AM_EXPECT(threads.count(Thread::GetCurrentThreadId()) == 1);

            const AmUInt64 before = source->GetBufferedFrames();
            thread.FillInline(source, 1000);
            AM_EXPECT(source->GetBufferedFrames() > before);
            AM_EXPECT_EQ(1u, decoder->GetCallingThreads().size());

            thread.Retire(source);
            AM_EXPECT(decoder->IsClosed());
        }
    };

    AM_REGISTER_TEST(mixer_streaming, streaming_thread_synchronous_fills_inline);
} // namespace SparkyStudios::Audio::Amplitude::Tests
