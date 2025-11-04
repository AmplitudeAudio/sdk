// Copyright (c) 2021-present Sparky Studios. All rights reserved.
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

#include <DSP/Filters/WaveShaperFilter.h>

#include "DSPTestCase.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    void DSPTestCase::Run()
    {
        auto filter = amshared(WaveShaperFilter);
        filter->Initialize(0.5f);

        auto instance = filter->CreateInstance();
        AM_EXPECT_NOT(instance == nullptr);

        // Create test audio buffers
        constexpr AmUInt64 frameCount = 1024;
        constexpr AmUInt16 channelCount = 2;
        constexpr AmUInt32 sampleRate = 48000;

        AudioBuffer inputBuffer(frameCount, channelCount);
        AudioBuffer outputBuffer(frameCount, channelCount);

        // Fill input buffer with test signal
        GenerateSineWave(inputBuffer, sampleRate);

        // Process audio
        instance->Process(inputBuffer, outputBuffer, frameCount, sampleRate);

        // Verify output buffer has non-zero values (shaped signal)
        AM_EXPECT(EnsureHasNonZeroOutput(outputBuffer));

        // Test parameter changes with different amounts
        instance->SetParameter(WaveShaperFilter::ATTRIBUTE_AMOUNT, -0.5f);
        AM_EXPECT(instance->GetParameter(WaveShaperFilter::ATTRIBUTE_AMOUNT) == -0.5f);

        outputBuffer.Clear();
        instance->Process(inputBuffer, outputBuffer, frameCount, sampleRate);
        AM_EXPECT(EnsureHasNonZeroOutput(outputBuffer));

        instance->SetParameter(WaveShaperFilter::ATTRIBUTE_AMOUNT, 1.0f);
        AM_EXPECT(instance->GetParameter(WaveShaperFilter::ATTRIBUTE_AMOUNT) == 1.0f);

        outputBuffer.Clear();
        instance->Process(inputBuffer, outputBuffer, frameCount, sampleRate);
        AM_EXPECT(EnsureHasNonZeroOutput(outputBuffer));

        // Test wet/dry mix
        instance->SetParameter(WaveShaperFilter::ATTRIBUTE_WET, 0.0f);
        AM_EXPECT(instance->GetParameter(WaveShaperFilter::ATTRIBUTE_WET) == 0.0f);

        outputBuffer.Clear();

        // With dry signal, output should equal input
        instance->Process(inputBuffer, outputBuffer, frameCount, sampleRate);

        AM_EXPECT(EnsureBufferEqual(inputBuffer, outputBuffer));
    }
} // namespace SparkyStudios::Audio::Amplitude::Tests
