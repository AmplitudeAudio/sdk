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

#include <cmath>
#include <vector>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Mixer/Voice/ResampleStream.h>

#include "DSPTestCase.h"
#include "TestRegistry.h"
#include "VoiceTestUtils.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(DSPTestCase, mixer_voice, resample_stream_reports_end_and_finish)
    {
    public:
        void Run() override
        {
            const AudioBuffer ramp = MakeRamp(1000, 0.001f);
            SourceReader reader;
            reader.Initialize(MemorySource(ramp, 48000), 0, 0, false, 0);

            ResampleStream stream;
            AM_EXPECT(stream.Initialize("default", 48000, 48000, 1, 256));

            AudioBuffer out(256, 1);
            ResampleStream::PullReport report;
            for (int pull = 0; pull < 3; ++pull)
            {
                report = stream.Pull(reader, out[0], 0, 256);
                AM_EXPECT_NOT(report.ended);
            }

            report = stream.Pull(reader, out[0], 0, 256); // frames 768..1023
            AM_EXPECT(report.ended);
            AM_EXPECT_EQ(232ULL, report.endFrame);
            AM_EXPECT(report.finished);
            AM_EXPECT(stream.IsFinished());
            AM_EXPECT_EQ(static_cast<AmReal32>(999) * 0.001f, out[0][231]);
            AM_EXPECT_EQ(0.0f, out[0][232]);
            AM_EXPECT_EQ(1000ULL, stream.GetSourcePosition(reader));
        }
    };

    AM_REGISTER_TEST(mixer_voice, resample_stream_reports_end_and_finish);
} // namespace SparkyStudios::Audio::Amplitude::Tests
