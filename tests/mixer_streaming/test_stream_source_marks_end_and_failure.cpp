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
    AM_TEST_CASE(ComponentTestCase, mixer_streaming, stream_source_marks_end_and_failure)
    {
    public:
        void Run() override
        {
            StreamPlan plan;
            plan.regionStart = 0;
            plan.regionEnd = 20;
            plan.start = 0;
            plan.target = 0;

            // A non-looping plan ends at the region end.
            {
                auto decoder = std::make_shared<StreamTestDecoder>(20, 2);
                StreamSource source(decoder, plan, 64, 2, 16, 48000, false);

                AM_EXPECT(!source.IsFilled());
                AM_EXPECT_EQ(20u, source.Fill(1000));
                AM_EXPECT_EQ(20u, source.GetRing().GetEndIndex());
                AM_EXPECT(!source.GetRing().IsFailed());
                AM_EXPECT(source.IsFilled());
                AM_EXPECT_EQ(0u, source.Fill(1000));
            }

            // A decoder that fails mid-stream.
            {
                auto decoder = std::make_shared<StreamTestDecoder>(20, 2);
                decoder->FailAt(12);
                StreamSource source(decoder, plan, 64, 2, 16, 48000, false);

                AM_EXPECT_EQ(12u, source.Fill(1000));
                AM_EXPECT(source.GetRing().IsFailed());
                AM_EXPECT(source.IsFilled());
            }

            // Destroying a source closes its decoder.
            {
                auto decoder = std::make_shared<StreamTestDecoder>(20, 2);
                {
                    StreamSource source(decoder, plan, 64, 2, 16, 48000, false);
                    AM_EXPECT(!decoder->IsClosed());
                }
                AM_EXPECT(decoder->IsClosed());
            }
        }
    };

    AM_REGISTER_TEST(mixer_streaming, stream_source_marks_end_and_failure);
} // namespace SparkyStudios::Audio::Amplitude::Tests
