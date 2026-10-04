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

#include "DSPTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(DSPTestCase, dsp_resamplers, resampler_contract_input_needed_is_minimal)
    {
    public:
        void Run() override
        {
            constexpr AmUInt32 kRates[][2] = { { 44100, 48000 }, { 48000, 44100 }, { 96000, 48000 } };

            for (const auto& rates : kRates)
            {
                auto instance = Resampler::Construct("default");
                instance->Initialize(1, rates[0], rates[1]);

                AM_EXPECT_EQ(0ULL, instance->GetInputFramesNeeded(0));

                const AmUInt64 needed = instance->GetInputFramesNeeded(512);
                AM_EXPECT(needed > 0);

                AudioBuffer input(needed, 1);
                AudioBuffer output(512, 1);
                AmUInt64 inFrames = needed - 1;
                AmUInt64 outFrames = 512;
                AM_EXPECT(instance->Process(input, inFrames, output, outFrames));

                // One frame short of what was asked for can never produce the full block.
                AM_EXPECT(outFrames < 512);
            }
        }
    };

    AM_REGISTER_TEST(dsp_resamplers, resampler_contract_input_needed_is_minimal);
} // namespace SparkyStudios::Audio::Amplitude::Tests
