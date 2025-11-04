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

#include <DSP/Delay.h>

#include "DSPTestCase.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    void DSPTestCase::Run()
    {
        constexpr AmSize maxDelay = 1000;
        constexpr AmSize framesCount = 256;

        Delay delay(maxDelay, framesCount);

        // Verify initialization
        AM_EXPECT(delay.GetMaxDelay() == maxDelay);
        AM_EXPECT(delay.GetDelayInSamples() == maxDelay + framesCount);

        // Create test channel
        AudioBuffer buffer(framesCount, 1);
        AudioBufferChannel& channel = buffer[0];

        // Fill with impulse
        channel[0] = 1.0f;
        for (AmSize i = 1; i < framesCount; ++i)
            channel[i] = 0.0f;

        // Insert into delay line
        delay.Insert(channel);

        // Create output channel
        AudioBuffer output(framesCount, 1);
        AudioBufferChannel& outChannel = output[0];

        // Process with delay
        constexpr AmSize delaySamples = 100;
        delay.Process(outChannel, delaySamples);

        // First output should be mostly zeros since we're reading delayed data
        AmSize firstValueIndex = 0;
        for (AmSize i = 0; i < framesCount; ++i)
        {
            if (std::abs(outChannel[i]) > kEpsilon)
            {
                firstValueIndex = i;
                break;
            }
        }

        AM_EXPECT(firstValueIndex == delaySamples); // First read should be zeros
    }
} // namespace SparkyStudios::Audio::Amplitude::Tests
