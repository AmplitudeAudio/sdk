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
    AM_TEST_CASE(ComponentTestCase, mixer_voice, source_reader_wraps_inside_a_read)
    {
    public:
        void Run() override
        {
            const AudioBuffer ramp = MakeRamp(10, 1.0f);
            SourceReader reader;
            reader.Initialize(MemorySource(ramp, 48000), 0, 0, true, 0);

            AudioBuffer out(25, 1);
            SourceReadReport report;
            AM_EXPECT_EQ(25ULL, reader.Read(out, 0, 25, report));
            AM_EXPECT_EQ(2U, report.wraps);
            AM_EXPECT_EQ(10ULL, report.firstWrapOffset);
            AM_EXPECT_NOT(report.ended);

            for (AmUInt64 i = 0; i < 25; ++i)
                AM_EXPECT_EQ(static_cast<AmReal32>(i % 10), out[0][i]);

            // A loop shorter than a block, read with a 4096-frame request, still terminates.
            AudioBuffer big(4096, 1);
            report = {};
            AM_EXPECT_EQ(4096ULL, reader.Read(big, 0, 4096, report));
            AM_EXPECT(report.wraps >= 409U);
        }
    };

    AM_REGISTER_TEST(mixer_voice, source_reader_wraps_inside_a_read);
} // namespace SparkyStudios::Audio::Amplitude::Tests
