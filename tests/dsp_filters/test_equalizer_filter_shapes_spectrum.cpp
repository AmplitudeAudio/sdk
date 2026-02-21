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
    AM_TEST_CASE(DSPTestCase, dsp_filters, equalizer_filter_shapes_spectrum)
    {
    public:
        void Run() override
        {
            constexpr AmUInt64 frameCount = 4096;
            constexpr AmUInt16 channelCount = 1;
            constexpr AmUInt32 sampleRate = 48000;

            // EQ with boosted low bands and cut high bands
            // Bands 1-4 boosted (2.0), bands 5-8 cut (0.3)
            auto filter = amshared(EqualizerFilter);
            filter->Initialize(2.0f, 2.0f, 2.0f, 2.0f, 0.3f, 0.3f, 0.3f, 0.3f);
            auto instance = filter->CreateInstance();
            AM_EXPECT_NOT(instance == nullptr);

            // Generate multi-frequency signal
            AudioBuffer input(frameCount, channelCount);
            GenerateMultiTone(input, sampleRate, { 100.0f, 500.0f, 2000.0f, 8000.0f }, 0.25f);

            AudioBuffer output(frameCount, channelCount);
            instance->Process(input, output, frameCount, sampleRate);

            // Output should not be silent
            ExpectNotSilent(output, 0.01f);

            // Output should differ from input (EQ is modifying the signal)
            AM_EXPECT_NOT(EnsureBufferEqual(input, output));

            // Compare with flat EQ (all bands at 1.0 = unity)
            auto flatFilter = amshared(EqualizerFilter);
            flatFilter->Initialize(1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
            auto flatInstance = flatFilter->CreateInstance();

            AudioBuffer flatOutput(frameCount, channelCount);
            flatInstance->Process(input, flatOutput, frameCount, sampleRate);

            // Shaped EQ output should differ from flat EQ output
            AM_EXPECT_NOT(EnsureBufferEqual(output, flatOutput));
        }
    };

    AM_REGISTER_TEST(dsp_filters, equalizer_filter_shapes_spectrum);
} // namespace SparkyStudios::Audio::Amplitude::Tests
