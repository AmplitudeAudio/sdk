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
        AmThreadHandle FailToCreateThread(AmThreadFunction, AmVoidPtr)
        {
            return nullptr;
        }
    } // namespace

    AM_TEST_CASE(ComponentTestCase, mixer_streaming, streaming_thread_falls_back_when_worker_fails)
    {
    public:
        void Run() override
        {
            StreamingThread thread({ 100.0, 20.0, false, 256 }, &FailToCreateThread);
            AM_EXPECT(!thread.IsSynchronous());

            // No worker: streams are filled by their consumer instead of starving after the prefill.
            thread.Start();
            AM_EXPECT(!thread.IsRunning());
            AM_EXPECT(thread.IsSynchronous());

            auto decoder = std::make_shared<StreamTestDecoder>(48000, 2);
            StreamPlan plan;
            plan.regionStart = 0;
            plan.regionEnd = 48000;

            StreamSource* source = thread.Create(decoder, plan, 2, 48000);
            AM_EXPECT(source->IsSynchronous());
            AM_EXPECT(thread.IsReady(source));

            const AmUInt64 before = source->GetBufferedFrames();
            thread.FillInline(source, 1000);
            AM_EXPECT(source->GetBufferedFrames() > before);

            thread.Retire(source);
            AM_EXPECT(decoder->IsClosed());
        }
    };

    AM_REGISTER_TEST(mixer_streaming, streaming_thread_falls_back_when_worker_fails);
} // namespace SparkyStudios::Audio::Amplitude::Tests
