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
#include <future>
#include <memory>
#include <thread>
#include <vector>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Mixer/Streaming/StreamSource.h>
#include <Mixer/Voice/SourceReader.h>

#include "ComponentTestCase.h"
#include "StreamTestDecoder.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    namespace
    {
        // Runs Fill on a detached thread that owns the source, so a regression that never returns fails the test
        // after one second instead of hanging it (a std::async future would block in its destructor).
        bool FillWithWatchdog(const std::shared_ptr<StreamSource>& source, AmUInt64& written)
        {
            auto promise = std::make_shared<std::promise<AmUInt64>>();
            auto future = promise->get_future();
            std::thread(
                [source, promise]()
                {
                    promise->set_value(source->Fill(1000));
                })
                .detach();

            if (future.wait_for(std::chrono::seconds(1)) != std::future_status::ready)
                return false;

            written = future.get();
            return true;
        }
    } // namespace

    AM_TEST_CASE(ComponentTestCase, mixer_streaming, stream_source_wraps_short_loops)
    {
    public:
        void Run() override
        {
            // A 3-frame loop looped forever, in a ring far larger than the loop.
            {
                auto decoder = std::make_shared<StreamTestDecoder>(100, 1);
                StreamPlan plan;
                plan.regionStart = 5;
                plan.regionEnd = 8;
                plan.loop = true;
                plan.remainingWraps = SourceReader::kInfiniteWraps;
                plan.start = 6;
                plan.target = 6;

                auto source = std::make_shared<StreamSource>(decoder, plan, 32, 1, 16, 48000, false);

                AmUInt64 written = 0;
                const bool returned = FillWithWatchdog(source, written);
                AM_EXPECT(returned);
                if (!returned)
                    return;

                AM_EXPECT_EQ(32u, written);

                const AmUInt64 pattern[] = { 6, 7, 5 };
                AudioBuffer out(32, 1);
                AM_EXPECT_EQ(32u, source->GetRing().Read(0, out, 0, 32));
                for (AmUInt64 i = 0; i < 32; ++i)
                    AM_EXPECT_EQ(StreamTestDecoder::ValueAt(pattern[i % 3], 0), out[0][i]);
            }

            // A finite plan: 3 frames, then 2 wraps of 3 frames each.
            {
                auto decoder = std::make_shared<StreamTestDecoder>(100, 1);
                StreamPlan plan;
                plan.regionStart = 5;
                plan.regionEnd = 8;
                plan.loop = true;
                plan.remainingWraps = 2;
                plan.start = 5;
                plan.target = 5;

                auto source = std::make_shared<StreamSource>(decoder, plan, 32, 1, 16, 48000, false);

                AmUInt64 written = 0;
                const bool returned = FillWithWatchdog(source, written);
                AM_EXPECT(returned);
                if (!returned)
                    return;

                AM_EXPECT_EQ(9u, written);
                AM_EXPECT_EQ(9u, source->GetRing().GetEndIndex());
                AM_EXPECT(source->IsFilled());
            }
        }
    };

    AM_REGISTER_TEST(mixer_streaming, stream_source_wraps_short_loops);
} // namespace SparkyStudios::Audio::Amplitude::Tests
