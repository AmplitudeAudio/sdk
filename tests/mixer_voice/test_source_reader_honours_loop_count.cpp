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

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Mixer/Voice/SourceReader.h>

#include "ComponentTestCase.h"
#include "TestRegistry.h"
#include "VoiceTestUtils.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(ComponentTestCase, mixer_voice, source_reader_honours_loop_count)
    {
    public:
        void Run() override
        {
            const AudioBuffer ramp = MakeRamp(10, 1.0f);
            SourceReader reader;
            reader.Initialize(MemorySource(ramp, 48000), 0, 0, true, 2); // two plays in total

            AudioBuffer out(30, 1);
            SourceReadReport report;
            AM_EXPECT_EQ(20ULL, reader.Read(out, 0, 30, report));
            AM_EXPECT_EQ(1U, report.wraps);
            AM_EXPECT(report.ended);
            AM_EXPECT_EQ(20ULL, report.endOffset);
            for (AmUInt64 i = 20; i < 30; ++i)
                AM_EXPECT_EQ(0.0f, out[0][i]);

            // An empty region never loops forever.
            SourceReader empty;
            empty.Initialize(MemorySource(ramp, 48000), 5, 5, true, 0);
            report = {};
            AM_EXPECT_EQ(0ULL, empty.Read(out, 0, 30, report));
            AM_EXPECT(empty.IsEnded());
        }
    };

    AM_REGISTER_TEST(mixer_voice, source_reader_honours_loop_count);
} // namespace SparkyStudios::Audio::Amplitude::Tests
