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
        AmVector3 location = { 10.0f, 20.0f, 30.0f };
        AmReal32 userGain = 0.36f;
        Channel channel = amEngine->Play(100, location, userGain);
        amEngine->WaitUntilFrames(2); // Playing is done in the next frame

        AM_EXPECT(channel.Valid());
        AM_EXPECT(channel.Playing());

        // Test that channel returns the correct initial gain
        AmReal32 result = channel.GetGain();
        AM_EXPECT_EQ(result, userGain);

        // Test updating gain
        AmReal32 newGain = 0.5f;
        channel.SetGain(newGain);

        // Test that channel returns the new gain
        AmReal32 result2 = channel.GetGain();
        AM_EXPECT_EQ(result2, newGain);

        channel.Stop(0);
    }
} // namespace SparkyStudios::Audio::Amplitude::Tests
