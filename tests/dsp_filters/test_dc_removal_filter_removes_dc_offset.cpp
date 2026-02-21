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

#include <DSP/Filters/DCRemovalFilter.h>

#include "DSPTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(DSPTestCase, dsp_filters, dc_removal_filter_removes_dc_offset)
    {
    public:
        void Run() override
        {
            constexpr AmUInt64 frameCount = 4096;
            constexpr AmUInt16 channelCount = 1;
            constexpr AmUInt32 sampleRate = 48000;

            auto filter = amshared(DCRemovalFilter);
            filter->Initialize(0.1f);
            auto instance = filter->CreateInstance();
            AM_EXPECT_NOT(instance == nullptr);

            // Generate sine wave with significant DC offset
            constexpr AmReal32 dcOffset = 0.5f;
            AudioBuffer input(frameCount, channelCount);
            GenerateSineWave(input, sampleRate, dcOffset);

            // Verify input actually has DC offset
            AmReal32 inputDC = CalculateDCOffset(input);
            AM_EXPECT(std::abs(inputDC - dcOffset) < 0.1f);

            // Process multiple passes to let the DC removal converge
            AudioBuffer output(frameCount, channelCount);
            for (int pass = 0; pass < 3; ++pass)
                instance->Process(input, output, frameCount, sampleRate);

            // DC offset should be significantly reduced
            AmReal32 outputDC = std::abs(CalculateDCOffset(output));
            AM_EXPECT(outputDC < std::abs(inputDC) * 0.5f);

            // AC signal should still be present
            ExpectNotSilent(output, 0.01f);
        }
    };

    AM_REGISTER_TEST(dsp_filters, dc_removal_filter_removes_dc_offset);
} // namespace SparkyStudios::Audio::Amplitude::Tests
