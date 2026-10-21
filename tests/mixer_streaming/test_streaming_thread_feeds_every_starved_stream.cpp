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

#include <algorithm>
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
    AM_TEST_CASE(ComponentTestCase, mixer_streaming, streaming_thread_feeds_every_starved_stream)
    {
    public:
        void Run() override
        {
            constexpr AmUInt32 kSources = 8;
            constexpr AmUInt64 kStep = 240; // 5 ms at 48 kHz

            StreamingThread thread({ 500.0, 20.0, false, 256 });

            std::vector<std::shared_ptr<StreamTestDecoder>> decoders;
            std::vector<StreamSource*> sources;
            StreamPlan plan;
            plan.regionStart = 0;
            plan.regionEnd = 48000 * 60;

            for (AmUInt32 i = 0; i < kSources; ++i)
            {
                auto decoder = std::make_shared<StreamTestDecoder>(48000 * 60, 2);
                decoder->SetDelay(2);
                decoders.push_back(decoder);
                sources.push_back(thread.Create(decoder, plan, 2, 48000));
            }

            thread.Start();

            // Consume every ring at real-time rate for one second. One worker cannot keep eight 2 ms reads real-time,
            // so the streams do starve: what matters is that the worker shares its calls fairly between them.
            std::vector<AmUInt64> consumed(kSources, 0);
            for (AmUInt32 tick = 0; tick < 200; ++tick)
            {
                Thread::Sleep(5);
                for (AmUInt32 i = 0; i < kSources; ++i)
                {
                    consumed[i] += kStep;
                    sources[i]->GetRing().Release(std::min(consumed[i], sources[i]->GetRing().GetWriteIndex()));
                }
            }

            thread.Stop();

            AmUInt64 minCalls = ~0ULL;
            AmUInt64 maxCalls = 0;
            for (const auto& decoder : decoders)
            {
                minCalls = std::min(minCalls, decoder->GetStreamCalls());
                maxCalls = std::max(maxCalls, decoder->GetStreamCalls());
            }

            // The test asserts fairness only, because absolute call counts depend on timer resolution
            // (Windows Sleep ticks are ~15.6 ms).
            AM_EXPECT(minCalls > 0u);
            AM_EXPECT(minCalls * 2 >= maxCalls);
        }
    };

    AM_REGISTER_TEST(mixer_streaming, streaming_thread_feeds_every_starved_stream);
} // namespace SparkyStudios::Audio::Amplitude::Tests
