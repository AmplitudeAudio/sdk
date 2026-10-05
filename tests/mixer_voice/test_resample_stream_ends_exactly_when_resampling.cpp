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
    AM_TEST_CASE(DSPTestCase, mixer_voice, resample_stream_ends_exactly_when_resampling)
    {
    public:
        void Run() override
        {
            // 1000 frames at 24 kHz on a 48 kHz output: the end lands on output frame 2000, and the filter tail is
            // flushed a few frames later.
            AudioBuffer constant(1000, 1);
            for (AmUInt64 i = 0; i < 1000; ++i)
                constant[0][i] = 0.5f;

            SourceReader reader;
            reader.Initialize(MemorySource(constant, 24000), 0, 0, false, 0);

            ResampleStream stream;
            AM_EXPECT(stream.Initialize("default", 24000, 48000, 1, 256));

            AudioBuffer out(256, 1);
            bool ended = false;
            bool finished = false;
            AmUInt64 endFrame = 0;
            AmUInt64 finishedFrame = 0;
            for (AmUInt64 pull = 0; pull < 12 && !finished; ++pull)
            {
                const ResampleStream::PullReport report = stream.Pull(reader, out[0], 0, 256);
                if (report.ended)
                {
                    ended = true;
                    endFrame = pull * 256 + report.endFrame;
                }

                if (report.finished)
                {
                    finished = true;
                    finishedFrame = pull * 256 + report.finishedFrame;
                }
            }

            AM_EXPECT(ended);
            AM_EXPECT(finished);
            AM_EXPECT(endFrame >= 1999 && endFrame <= 2000);
            AM_EXPECT(finishedFrame >= endFrame && finishedFrame <= endFrame + 2 * 41 + 2);
        }
    };

    AM_REGISTER_TEST(mixer_voice, resample_stream_ends_exactly_when_resampling);
} // namespace SparkyStudios::Audio::Amplitude::Tests