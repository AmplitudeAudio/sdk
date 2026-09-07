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

#include <DSP/Filters/EqualizerFilter.h>

#include "DSPTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(DSPTestCase, dsp_filters, equalizer_filter_flat_response_is_transparent)
    {
    public:
        void Run() override
        {
            constexpr AmUInt64 frameCount = 4096;
            constexpr AmUInt16 channelCount = 1;
            constexpr AmUInt32 sampleRate = 48000;

            auto filter = amshared(EqualizerFilter);
            filter->Initialize(1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
            auto instance = filter->CreateInstance();
            AM_EXPECT_NOT(instance == nullptr);

            AudioBuffer input(frameCount, channelCount);
            AudioBuffer output(frameCount, channelCount);

            // Includes 15 kHz content to cover the full processed spectrum.
            GenerateMultiTone(input, sampleRate, { 100.0f, 500.0f, 2000.0f, 8000.0f, 15000.0f }, 0.15f);

            instance->Process(input, output, frameCount, sampleRate);

            // A flat EQ must be near-transparent: output == input within fp error.
            for (AmUInt64 i = 0; i < frameCount; ++i)
                AM_EXPECT(std::abs(output[0][i] - input[0][i]) < 1e-3f);
        }
    };

    AM_REGISTER_TEST(dsp_filters, equalizer_filter_flat_response_is_transparent);
} // namespace SparkyStudios::Audio::Amplitude::Tests
