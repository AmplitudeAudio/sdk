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

#include "SimpleTestCase.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    void SimpleTestCase::Run()
    {
        constexpr AmSize initialMaxDelay = 200;
        constexpr AmSize framesCount = 128;

        Delay delay(initialMaxDelay, framesCount);

        AM_EXPECT(delay.GetMaxDelay() == initialMaxDelay);

        // Update max delay
        constexpr AmSize newMaxDelay = 500;
        delay.SetMaxDelay(newMaxDelay);

        AM_EXPECT(delay.GetMaxDelay() == newMaxDelay);
        AM_EXPECT(delay.GetDelayInSamples() == newMaxDelay + framesCount);
    }
} // namespace SparkyStudios::Audio::Amplitude::Tests
