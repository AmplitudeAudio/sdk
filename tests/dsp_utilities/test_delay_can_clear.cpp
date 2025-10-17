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
        constexpr AmSize maxDelay = 500;
        constexpr AmSize framesCount = 128;

        Delay delay(maxDelay, framesCount);

        // Create and insert test data
        AudioBuffer buffer(framesCount, 1);
        AudioBufferChannel& channel = buffer[0];

        for (AmSize i = 0; i < framesCount; ++i)
            channel[i] = 1.0f;

        delay.Insert(channel);

        // Clear the delay line
        delay.Clear();

        // Process and verify output is zeros
        AudioBuffer output(framesCount, 1);
        AudioBufferChannel& outChannel = output[0];

        delay.Process(outChannel, 10);

        AM_EXPECT(EnsureHasZeroOutput(output));
    }
} // namespace SparkyStudios::Audio::Amplitude::Tests
