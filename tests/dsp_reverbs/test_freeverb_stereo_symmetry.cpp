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
#include <Utils/Freeverb/ReverbModel.h>

#include "DSPTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(DSPTestCase, dsp_reverbs, freeverb_stereo_symmetry)
    {
    public:
        void Run() override
        {
            Freeverb::ReverbModel model;
            model.SetWidth(1.0f);
            model.SetWet(1.0f);
            model.SetDry(0.0f);
            model.SetRoomSize(0.8f);
            model.SetDamp(0.2f);

            constexpr AmUInt64 frameCount = 4096;
            AudioBuffer in(frameCount, 2);
            AudioBuffer out(frameCount, 2);

            // Feed identical impulse on L and R
            in[0][0] = 1.0f;
            in[1][0] = 1.0f;

            model.ProcessReplace(in[0].begin(), in[1].begin(), out[0].begin(), out[1].begin(), frameCount, 1);

            AmReal32 energyL = 0.0f;
            AmReal32 energyR = 0.0f;
            for (AmUInt64 f = 0; f < frameCount; ++f)
            {
                energyL += out[0][f] * out[0][f];
                energyR += out[1][f] * out[1][f];
            }

            // For identical stereo input, energy in L and R should be closely matched (ratio within 10%)
            AmReal32 ratio = energyL / energyR;
            AM_EXPECT(ratio > 0.85f && ratio < 1.15f);
        }
    };

    AM_REGISTER_TEST(dsp_reverbs, freeverb_stereo_symmetry);
} // namespace SparkyStudios::Audio::Amplitude::Tests
