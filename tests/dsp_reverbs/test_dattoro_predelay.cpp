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
#include <Utils/Audio/Reverb/DattoroReverb.h>

#include "DSPTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(DSPTestCase, dsp_reverbs, dattoro_predelay)
    {
    public:
        void Run() override
        {
            DattoroReverb reverb(48000);

            constexpr AmUInt64 frameCount = 8000;
            AudioBuffer in(frameCount, 1);
            AudioBuffer out(frameCount, 2);

            in[0][0] = 1.0f; // Single impulse

            reverb.Process(in, out);

            // Output should remain silent during early frames before predelay expires (< 4000)
            for (AmUInt64 f = 0; f < 4000; ++f)
            {
                AM_EXPECT_EQ(out[0][f], 0.0f);
                AM_EXPECT_EQ(out[1][f], 0.0f);
            }

            // Output after the predelay onset should have non-zero reverberant signal
            AmReal32 lateEnergy = 0.0f;
            for (AmUInt64 f = 4000; f < frameCount; ++f)
            {
                lateEnergy += out[0][f] * out[0][f] + out[1][f] * out[1][f];
            }

            AM_EXPECT(lateEnergy > 0.001f);
        }
    };

    AM_REGISTER_TEST(dsp_reverbs, dattoro_predelay);
} // namespace SparkyStudios::Audio::Amplitude::Tests
