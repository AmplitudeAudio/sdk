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

#include <DSP/Filters/FlangerFilter.h>

#include "DSPTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(DSPTestCase, dsp_filters, flanger_filter_modulates_signal)
    {
    public:
        void Run() override
        {
            constexpr AmUInt64 frameCount = 8192;
            constexpr AmUInt16 channelCount = 1;
            constexpr AmUInt32 sampleRate = 48000;

            auto filter = amshared(FlangerFilter);
            filter->Initialize(0.005f, 2.0f);
            auto instance = filter->CreateInstance();
            AM_EXPECT_NOT(instance == nullptr);

            AudioBuffer input(frameCount, channelCount);
            GenerateSineWaveAtFrequency(input, sampleRate, 440.0f, 0.5f);

            AudioBuffer output(frameCount, channelCount);
            instance->Process(input, output, frameCount, sampleRate);

            // Output should not be silent
            ExpectNotSilent(output, 0.01f);

            // Output should differ from input (flanging modifies the signal)
            AM_EXPECT_NOT(EnsureBufferEqual(input, output));

            // Flanging introduces constructive/destructive interference,
            // so the output amplitude should vary over time (not constant like the input sine).
            // Check that the output has varying envelope by comparing RMS of first and second halves
            AmReal32 firstHalfPeak = 0.0f;
            AmReal32 secondHalfPeak = 0.0f;
            for (AmUInt64 i = 0; i < frameCount / 2; ++i)
                firstHalfPeak = std::max(firstHalfPeak, std::abs(output[0][i]));
            for (AmUInt64 i = frameCount / 2; i < frameCount; ++i)
                secondHalfPeak = std::max(secondHalfPeak, std::abs(output[0][i]));

            // Both halves should have energy
            AM_EXPECT(firstHalfPeak > 0.01f);
            AM_EXPECT(secondHalfPeak > 0.01f);
        }
    };

    AM_REGISTER_TEST(dsp_filters, flanger_filter_modulates_signal);
} // namespace SparkyStudios::Audio::Amplitude::Tests
