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

#include <DSP/Filters/BassBoostFilter.h>

#include "DSPTestCase.h"
#include "TestRegistry.h"
#include "TestUtils.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(DSPTestCase, dsp_filters, fft_filter_reuses_plan_across_blocks)
    {
    public:
        void Run() override
        {
            constexpr AmUInt64 frameCount = 1024; // 4 STFT blocks of 256 samples
            constexpr AmUInt16 channelCount = 2;
            constexpr AmUInt32 sampleRate = 48000;

            AudioBuffer inputBuffer(frameCount, channelCount);
            GenerateSineWave(inputBuffer, sampleRate);

            AudioBuffer outputA(frameCount, channelCount);
            AudioBuffer outputB(frameCount, channelCount);

            auto filterA = amshared(BassBoostFilter);
            filterA->Initialize(2.0f);
            auto instanceA = filterA->CreateInstance();
            instanceA->Process(inputBuffer, outputA, frameCount, sampleRate);

            auto filterB = amshared(BassBoostFilter);
            filterB->Initialize(2.0f);
            auto instanceB = filterB->CreateInstance();
            instanceB->Process(inputBuffer, outputB, frameCount, sampleRate);

            // A cached plan must not change results: two instances produce identical output
            AM_EXPECT(EnsureBufferEqual(outputA, outputB));

            // Sanity: output is non-zero and differs from the input
            AM_EXPECT(EnsureHasNonZeroOutput(outputA));
            AM_EXPECT_NOT(EnsureBufferEqual(inputBuffer, outputA));
        }
    };

    AM_REGISTER_TEST(dsp_filters, fft_filter_reuses_plan_across_blocks);
} // namespace SparkyStudios::Audio::Amplitude::Tests
