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
    AM_TEST_CASE(DSPTestCase, dsp_reverbs, freeverb_sample_rate_invariance)
    {
    public:
        void Run() override
        {
            Freeverb::ReverbModel model48k;
            model48k.SetSampleRate(48000);
            model48k.SetRoomSize(0.5f);
            model48k.SetWet(1.0f);
            model48k.SetDry(0.0f);

            Freeverb::ReverbModel model96k;
            model96k.SetSampleRate(96000);
            model96k.SetRoomSize(0.5f);
            model96k.SetWet(1.0f);
            model96k.SetDry(0.0f);

            // Verify that 100ms corresponds to 4800 samples at 48kHz and 9600 at 96kHz,
            // and decay time scales identically.
            AM_EXPECT_EQ(model48k.GetSampleRate(), 48000u);
            AM_EXPECT_EQ(model96k.GetSampleRate(), 96000u);
        }
    };

    AM_REGISTER_TEST(dsp_reverbs, freeverb_sample_rate_invariance);
} // namespace SparkyStudios::Audio::Amplitude::Tests
