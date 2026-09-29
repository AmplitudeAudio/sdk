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
    AM_TEST_CASE(ComponentTestCase, mixer_voice, source_reader_rewind_wraps)
    {
    public:
        void Run() override
        {
            const AudioBuffer ramp = MakeRamp(10, 1.0f);
            SourceReader reader;
            reader.Initialize(MemorySource(ramp, 48000), 0, 0, true, 0);

            AudioBuffer out(13, 1);
            SourceReadReport report;
            reader.Read(out, 0, 13, report);
            AM_EXPECT_EQ(3ULL, reader.GetCursor());
            AM_EXPECT_EQ(1ULL, reader.Rewind(2));
            AM_EXPECT_EQ(8ULL, reader.Rewind(5));
            AM_EXPECT_EQ(3ULL, reader.Rewind(10));

            SourceReader once;
            once.Initialize(MemorySource(ramp, 48000), 0, 0, false, 0);
            once.Seek(4);
            AM_EXPECT_EQ(0ULL, once.Rewind(9)); // no loop: clamps at the region start
        }
    };

    AM_REGISTER_TEST(mixer_voice, source_reader_rewind_wraps);
} // namespace SparkyStudios::Audio::Amplitude::Tests
