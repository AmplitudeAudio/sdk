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
    AM_TEST_CASE(DSPTestCase, mixer_voice, resample_stream_downmixes_to_mono)
    {
    public:
        void Run() override
        {
            AudioBuffer stereo(64, 2);
            for (AmUInt64 i = 0; i < 64; ++i)
            {
                stereo[0][i] = 0.5f;
                stereo[1][i] = 0.25f;
            }

            SourceReader reader;
            reader.Initialize(MemorySource(stereo, 48000), 0, 0, true, 0);

            ResampleStream stream;
            AM_EXPECT(stream.Initialize("default", 48000, 48000, 2, 64));

            AudioBuffer out(64, 1);
            AM_UNUSED(stream.Pull(reader, out[0], 0, 64));

            // Same downmix as AudioConverter::ConvertMonoFromStereo: (L + R) / sqrt(2). InverseSquareRoot() is a fast
            // approximation, accurate to about 1.33e-4 for this input: the tolerance covers that error.
            const AmReal32 expected = (0.5f + 0.25f) / std::sqrt(2.0f);
            for (AmUInt64 i = 0; i < 64; ++i)
                AM_EXPECT(std::abs(out[0][i] - expected) < 2e-4f);
        }
    };

    AM_REGISTER_TEST(mixer_voice, resample_stream_downmixes_to_mono);
} // namespace SparkyStudios::Audio::Amplitude::Tests
