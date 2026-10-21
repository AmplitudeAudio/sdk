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
    AM_TEST_CASE(ComponentTestCase, mixer_streaming, streaming_thread_picks_up_sources_mid_pass)
    {
    public:
        void Run() override
        {
            constexpr int kSlowSources = 6;
            constexpr AmInt32 kDelayMs = 40;

            StreamPlan plan;
            plan.regionStart = 0;
            plan.regionEnd = 48000 * 10;

            // Capacity 4800 frames in chunks of 256: a full pass takes ~19 rounds of kSlowSources * kDelayMs.
            StreamingThread thread({ 100.0, 20.0, false, 256 });
            thread.Start();

            std::vector<std::shared_ptr<StreamTestDecoder>> slow;
            std::vector<StreamSource*> slowSources;
            for (int i = 0; i < kSlowSources; ++i)
            {
                slow.push_back(std::make_shared<StreamTestDecoder>(48000 * 10, 2));
                slow.back()->SetDelay(kDelayMs);
                slowSources.push_back(thread.Create(slow.back(), plan, 2, 48000));
            }

            // Let the worker get well into its first pass.
            for (int i = 0; i < 2000 && slow[0]->GetStreamCalls() < 3; ++i)
                Thread::Sleep(1);
            AM_EXPECT(slow[0]->GetStreamCalls() >= 3);

            auto fresh = std::make_shared<StreamTestDecoder>(48000 * 10, 2);
            StreamSource* source = thread.Create(fresh, plan, 2, 48000);

            bool ready = false;
            for (int i = 0; i < 10000 && !ready; ++i)
            {
                ready = thread.IsReady(source);
                if (!ready)
                    Thread::Sleep(1);
            }
            AM_EXPECT(ready);

            // It became ready while the slow sources were still being filled: the pass did not have to end first.
            bool allFilled = true;
            for (auto* s : slowSources)
                allFilled = allFilled && s->IsFilled();
            AM_EXPECT(!allFilled);

            thread.Stop();
            for (auto* s : slowSources)
                thread.Retire(s);
            thread.Retire(source);
            thread.CollectRetired();
        }
    };

    AM_REGISTER_TEST(mixer_streaming, streaming_thread_picks_up_sources_mid_pass);
} // namespace SparkyStudios::Audio::Amplitude::Tests
