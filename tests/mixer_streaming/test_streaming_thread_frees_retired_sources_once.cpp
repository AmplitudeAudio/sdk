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
#include <vector>

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
        // Counts the decoders closed so far.
        AmUInt32 ClosedCount(const std::vector<std::shared_ptr<StreamTestDecoder>>& decoders)
        {
            AmUInt32 count = 0;
            for (const auto& decoder : decoders)
                count += decoder->IsClosed() ? 1 : 0;
            return count;
        }
    } // namespace

    AM_TEST_CASE(ComponentTestCase, mixer_streaming, streaming_thread_frees_retired_sources_once)
    {
    public:
        void Run() override
        {
            StreamPlan plan;
            plan.regionStart = 0;
            plan.regionEnd = 48000;

            StreamingThread thread({ 100.0, 20.0, false, 256 });
            thread.Start();

            std::vector<std::shared_ptr<StreamTestDecoder>> decoders;
            std::vector<StreamSource*> sources;
            for (int i = 0; i < 3; ++i)
            {
                decoders.push_back(std::make_shared<StreamTestDecoder>(48000, 2));
                sources.push_back(thread.Create(decoders.back(), plan, 2, 48000));
            }

            // Retire in a different order, across passes. Nothing is closed before an acknowledgement.
            thread.Retire(sources[1]);
            // Retire defers the free: the worker may still hold the source until it acknowledges.
            AM_EXPECT(!decoders[1]->IsClosed());
            thread.Retire(sources[1]); // a second retire of the same source is ignored
            for (int i = 0; i < 200 && !decoders[1]->IsClosed(); ++i)
            {
                thread.CollectRetired();
                Thread::Sleep(1);
            }
            AM_EXPECT(decoders[1]->IsClosed());
            AM_EXPECT(!decoders[0]->IsClosed());
            AM_EXPECT(!decoders[2]->IsClosed());

            thread.Retire(sources[2]);
            thread.Retire(sources[0]);
            for (int i = 0; i < 200 && ClosedCount(decoders) < 3; ++i)
            {
                thread.CollectRetired();
                Thread::Sleep(1);
            }
            AM_EXPECT_EQ(3u, ClosedCount(decoders));

            // Further collections free nothing more (an extra free would crash or double close).
            thread.CollectRetired();
            thread.CollectRetired();

            // With the worker stopped, a retire frees right away.
            thread.Stop();
            auto decoder = std::make_shared<StreamTestDecoder>(48000, 2);
            StreamSource* late = thread.Create(decoder, plan, 2, 48000);
            AM_EXPECT(!decoder->IsClosed());
            thread.Retire(late);
            AM_EXPECT(decoder->IsClosed());

            // A source left registered is freed by the destructor.
            auto remaining = std::make_shared<StreamTestDecoder>(48000, 2);
            {
                StreamingThread other({ 100.0, 20.0, false, 256 });
                other.Create(remaining, plan, 2, 48000);
                AM_EXPECT(!remaining->IsClosed());
            }
            AM_EXPECT(remaining->IsClosed());
        }
    };

    AM_REGISTER_TEST(mixer_streaming, streaming_thread_frees_retired_sources_once);
} // namespace SparkyStudios::Audio::Amplitude::Tests
