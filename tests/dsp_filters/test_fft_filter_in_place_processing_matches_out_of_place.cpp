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
    AM_TEST_CASE(DSPTestCase, dsp_filters, fft_filter_in_place_processing_matches_out_of_place)
    {
    public:
        void Run() override
        {
            constexpr AmUInt64 frameCount = 4096;
            constexpr AmUInt16 channelCount = 2;
            constexpr AmUInt32 sampleRate = 48000;

            auto filter = amshared(EqualizerFilter);
            filter->Initialize(1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);

            auto outOfPlaceInstance = filter->CreateInstance();
            auto inPlaceInstance = filter->CreateInstance();
            AM_EXPECT_NOT(outOfPlaceInstance == nullptr);
            AM_EXPECT_NOT(inPlaceInstance == nullptr);

            AudioBuffer input(frameCount, channelCount);
            GenerateMultiTone(input, sampleRate, { 100.0f, 1000.0f, 6000.0f }, 0.2f);

            // Run A: distinct input/output buffers (zero-copy path).
            AudioBuffer outOfPlaceOutput(frameCount, channelCount);
            outOfPlaceInstance->Process(input, outOfPlaceOutput, frameCount, sampleRate);

            // Run B: same buffer used as both input and output (aliased path),
            // through a fresh instance so both engines start from identical state.
            AudioBuffer inPlaceBuffer = input.Clone();
            inPlaceInstance->Process(inPlaceBuffer, inPlaceBuffer, frameCount, sampleRate);

            // Staging copies the input before any output is written, so both
            // paths must be numerically identical for the full buffer length.
            for (AmUInt16 c = 0; c < channelCount; ++c)
                for (AmUInt64 i = 0; i < frameCount; ++i)
                    AM_EXPECT(std::abs(inPlaceBuffer[c][i] - outOfPlaceOutput[c][i]) < 1e-4f);

            // Guard against a vacuous pass: past the 256-sample latency the
            // in-place output must actually contain the processed signal.
            AmReal64 sumSquares = 0.0;
            AmUInt64 count = 0;
            for (AmUInt16 c = 0; c < channelCount; ++c)
            {
                for (AmUInt64 i = 256; i < frameCount; ++i)
                {
                    const AmReal64 sample = inPlaceBuffer[c][i];
                    sumSquares += sample * sample;
                    ++count;
                }
            }

            AM_EXPECT(std::sqrt(sumSquares / static_cast<AmReal64>(count)) > 0.0);
        }
    };

    AM_REGISTER_TEST(dsp_filters, fft_filter_in_place_processing_matches_out_of_place);
} // namespace SparkyStudios::Audio::Amplitude::Tests
