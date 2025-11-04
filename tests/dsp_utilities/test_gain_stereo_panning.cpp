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

#include <DSP/Gain.h>

#include "SimpleTestCase.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    void SimpleTestCase::Run()
    {
        constexpr AmReal32 gain = 1.0f;

        // Test center pan (0.0)
        AmVector2 centerGains = Gain::CalculateStereoPannedGain(gain, 0.0f);
        AM_EXPECT(std::abs(centerGains.x - centerGains.y) < 0.01f); // Left and right should be equal

        // Test full left (-1.0)
        AmVector2 leftGains = Gain::CalculateStereoPannedGain(gain, -1.0f);
        AM_EXPECT(leftGains.x > leftGains.y); // Left should be stronger

        // Test full right (+1.0)
        AmVector2 rightGains = Gain::CalculateStereoPannedGain(gain, 1.0f);
        AM_EXPECT(rightGains.y > rightGains.x); // Right should be stronger

        // Test with zero gain
        AmVector2 zeroGains = Gain::CalculateStereoPannedGain(0.0f, 0.0f);
        AM_EXPECT(Gain::IsZero(zeroGains.x));
        AM_EXPECT(Gain::IsZero(zeroGains.y));

        // Test IsZero and IsOne utilities
        AM_EXPECT(Gain::IsZero(0.0f));
        AM_EXPECT(Gain::IsZero(0.0001f));
        AM_EXPECT_NOT(Gain::IsZero(0.1f));

        AM_EXPECT(Gain::IsOne(1.0f));
        AM_EXPECT(Gain::IsOne(0.9999f));
        AM_EXPECT_NOT(Gain::IsOne(0.5f));
    }
} // namespace SparkyStudios::Audio::Amplitude::Tests
