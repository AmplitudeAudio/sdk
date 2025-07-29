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

#include "EngineTestCase.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    void EngineTestCase::Run()
    {
        Bus bus = amEngine->FindBus(kAmMasterBusId);
        AM_EXPECT(bus.Valid());

        // Set initial gain
        bus.SetGain(1.0f);

        // Test fading to lower value
        bus.FadeTo(0.5f, kMinFadeDuration);
        Thread::Sleep(kAmSecond);
        AM_EXPECT_EQ(bus.GetGain(), 0.5f);

        // Test fading to higher value
        bus.FadeTo(1.0f, kMinFadeDuration);
        Thread::Sleep(kAmSecond);
        AM_EXPECT_EQ(bus.GetGain(), 1.0f);
    }
} // namespace SparkyStudios::Audio::Amplitude::Tests
