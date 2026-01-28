// Copyright (c) 2024-present Sparky Studios. All rights reserved.
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

#include <DSP/Gain.h>

#include "DSPTestCase.h"
#include "TestRegistry.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(DSPTestCase, dsp_utilities, gain_processor)
    {
    public:
        void Run() override
        {
            constexpr AmSize frameCount = 256;
            AudioBuffer input(frameCount, 1);
            AudioBuffer output(frameCount, 1);

            // Fill input
            for (AmSize i = 0; i < frameCount; ++i)
            input[0][i] = 1.0f;

            // Create gain processor with initial gain
            GainProcessor processor(0.0f);
            AM_EXPECT(std::abs(processor.GetGain() - 0.0f) < kEpsilon);

            // Apply gain with ramping
            processor.ApplyGain(1.0f, input[0], 0, output[0], 0, frameCount, false);

            // Verify output has non-zero values (ramped from 0 to 1)
            AM_EXPECT(EnsureHasNonZeroOutput(output));

            // Current gain should be at target
            AM_EXPECT(std::abs(processor.GetGain() - (frameCount / 2048.0f)) < 0.01f);

            // Test SetGain
            processor.SetGain(0.5f);
            AM_EXPECT(std::abs(processor.GetGain() - 0.5f) < kEpsilon);
        }
    };

    AM_REGISTER_TEST(dsp_utilities, gain_processor);
} // namespace SparkyStudios::Audio::Amplitude::Tests
