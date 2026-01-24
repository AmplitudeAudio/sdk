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

#include <DSP/Filters/BiquadResonantFilter.h>
#include <DSP/NearFieldProcessor.h>

#include "DSPTestCase.h"
#include "TestRegistry.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(DSPTestCase, dsp_utilities, near_field_processor_with_hrtf)
    {
    public:
        void Run() override
        {
            auto biquadFilter = Engine::RegisterExtension<BiquadResonantFilter>();

            constexpr AmUInt32 sampleRate = 48000;
            constexpr AmUInt32 framesCount = 512;

            NearFieldProcessor processor(sampleRate, framesCount);

            // Create buffers
            AudioBuffer input(framesCount, 1);
            AudioBuffer outputWithoutHrtf(framesCount, 1);
            AudioBuffer outputWithHrtf(framesCount, 1);

            // Generate test signal
            for (AmSize i = 0; i < framesCount; ++i)
            input[0][i] = std::sin(2.0f * AM_PI32 * 1000.0f * static_cast<AmReal32>(i) / static_cast<AmReal32>(sampleRate));

            // Process without HRTF
            processor.Process(input[0], outputWithoutHrtf[0], false);

            // Process with HRTF (includes delay compensation)
            processor.Process(input[0], outputWithHrtf[0], true);

            // Both outputs should have non-zero values
            AM_EXPECT(EnsureHasNonZeroOutput(outputWithoutHrtf));
            AM_EXPECT(EnsureHasNonZeroOutput(outputWithHrtf));

            // Outputs with and without HRTF should differ due to delay compensation
            AM_EXPECT_NOT(EnsureBufferEqual(outputWithoutHrtf, outputWithHrtf));

            Engine::UnregisterExtension(biquadFilter);
        }
    };

    AM_REGISTER_TEST(dsp_utilities, near_field_processor_with_hrtf);
} // namespace SparkyStudios::Audio::Amplitude::Tests
