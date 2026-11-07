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
    AM_TEST_CASE(ComponentTestCase, mixer_streaming, streaming_thread_floors_ring_capacity)
    {
    public:
        void Run() override
        {
            // 20 ms buffer and 5 ms prefill at 48 kHz: 960 frames, less than one 2048-frame voice block reads.
            StreamingThread thread({ 20.0, 5.0, true, 256 });

            StreamPlan plan;
            plan.regionStart = 0;
            plan.regionEnd = 48000;

            auto unfloored = std::make_shared<StreamTestDecoder>(48000, 2);
            StreamSource* small = thread.Create(unfloored, plan, 2, 48000);
            AM_EXPECT_EQ(960u, small->GetRing().GetCapacity());
            thread.Retire(small);

            // The consumer's floor wins over a configuration too small to hold one block.
            auto floored = std::make_shared<StreamTestDecoder>(48000, 2);
            StreamSource* source = thread.Create(floored, plan, 2, 48000, 8192);
            AM_EXPECT(source->GetRing().GetCapacity() >= 8192u);

            // A synchronous fill can now hold a whole block.
            thread.FillInline(source, 8192);
            AM_EXPECT(source->GetBufferedFrames() >= 2048u);
            thread.Retire(source);

            // A floor below the configured capacity changes nothing.
            auto generous = std::make_shared<StreamTestDecoder>(48000, 2);
            StreamSource* large = thread.Create(generous, plan, 2, 48000, 16);
            AM_EXPECT_EQ(960u, large->GetRing().GetCapacity());
            thread.Retire(large);
        }
    };

    AM_REGISTER_TEST(mixer_streaming, streaming_thread_floors_ring_capacity);
} // namespace SparkyStudios::Audio::Amplitude::Tests
