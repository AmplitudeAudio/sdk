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

#include <Mixer/Voice/ResampleStream.h>
#include <Mixer/Voice/SourceReader.h>

#include "DSPTestCase.h"
#include "TestRegistry.h"
#include "VoiceTestUtils.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(DSPTestCase, mixer_voice, resample_stream_handles_ratios_above_the_cap)
    {
    public:
        void Run() override
        {
            // A 96 kHz source pitched up 4x on a 48 kHz output: 8 source frames per output frame, twice the stretch cap.
            const AudioBuffer ramp = MakeRamp(96000, 1.0f / 96000.0f);
            SourceReader reader;
            reader.Initialize(MemorySource(ramp, 96000), 0, 0, true, 0);

            ResampleStream stream;
            AM_EXPECT(stream.Initialize("default", 96000, 48000, 1, 1024));
            stream.SetSpeed(4.0);
            AM_EXPECT_EQ(8.0, stream.GetRatio());

            AudioBuffer out(1024, 1);
            for (int pull = 0; pull < 20; ++pull)
            {
                const ResampleStream::PullReport report = stream.Pull(reader, out[0], 0, 1024);
                AM_EXPECT_EQ(1024ULL, report.produced);
                AM_EXPECT_NOT(report.error);
            }
        }
    };

    AM_REGISTER_TEST(mixer_voice, resample_stream_handles_ratios_above_the_cap);
} // namespace SparkyStudios::Audio::Amplitude::Tests
