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
    AM_TEST_CASE(DSPTestCase, mixer_voice, resample_stream_maps_loop_wraps)
    {
    public:
        void Run() override
        {
            const AudioBuffer ramp = MakeRamp(100, 1.0f);
            SourceReader reader;
            reader.Initialize(MemorySource(ramp, 48000), 0, 0, true, 0);

            ResampleStream stream;
            AM_EXPECT(stream.Initialize("default", 48000, 48000, 1, 256));

            AudioBuffer out(256, 1);
            const auto report = stream.Pull(reader, out[0], 0, 256);
            AM_EXPECT_EQ(2U, report.wraps);
            AM_EXPECT_EQ(100ULL, report.firstWrapFrame);
            AM_EXPECT_EQ(99.0f, out[0][99]);
            AM_EXPECT_EQ(0.0f, out[0][100]);
            AM_EXPECT_EQ(55.0f, out[0][255]);
        }
    };

    AM_REGISTER_TEST(mixer_voice, resample_stream_maps_loop_wraps);
} // namespace SparkyStudios::Audio::Amplitude::Tests
