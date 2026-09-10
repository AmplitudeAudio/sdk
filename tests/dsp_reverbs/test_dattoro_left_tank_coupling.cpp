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
    AM_TEST_CASE(DSPTestCase, dsp_reverbs, dattoro_left_tank_coupling)
    {
    public:
        void Run() override
        {
            DattoroReverb reverb(48000);

            constexpr AmUInt64 frameCount = 16384;
            AudioBuffer in(frameCount, 1);
            AudioBuffer out(frameCount, 2);

            in[0][0] = 1.0f; // Single impulse

            reverb.Process(in, out);

            // Left tank taps (e.g. tap 2, 3, 1) arrive between frame 5000 and 8000.
            // Right-tank taps in left channel (taps 4, 5, 6) only arrive after frame 8900.
            // Therefore, energy in out[0] before frame 8000 directly verifies left tank coupling.
            AmReal32 leftTankEnergy = 0.0f;
            for (AmUInt64 f = 5000; f < 8000; ++f)
            {
                leftTankEnergy += out[0][f] * out[0][f];
            }

            AM_EXPECT(leftTankEnergy > 0.001f);

            // Left channel (out[0]) and Right channel (out[1]) must both have active reverberant energy overall
            AmReal32 energyL = 0.0f;
            AmReal32 energyR = 0.0f;
            for (AmUInt64 f = 0; f < frameCount; ++f)
            {
                energyL += out[0][f] * out[0][f];
                energyR += out[1][f] * out[1][f];
            }

            AM_EXPECT(energyL > 0.001f);
            AM_EXPECT(energyR > 0.001f);
        }
    };

    AM_REGISTER_TEST(dsp_reverbs, dattoro_left_tank_coupling);
} // namespace SparkyStudios::Audio::Amplitude::Tests
