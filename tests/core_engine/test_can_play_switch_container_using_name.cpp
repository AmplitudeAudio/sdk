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
        Entity entity = amEngine->AddEntity(100);

        Channel channel = amEngine->Play("footsteps");
        AM_EXPECT_NOT(channel.Valid()); // switch container is entity scoped

        channel = amEngine->Play("footsteps", entity);
        amEngine->WaitUntilNextFrame(); // Playing is done in the next frame

        AM_EXPECT(channel.Valid());
        AM_EXPECT(channel.Playing());

        Thread::Sleep(1000); // wait for the sound to finish playing
        AM_EXPECT_NOT(channel.Playing());

        channel.Stop(0);

        // Clean up
        amEngine->RemoveEntity(100);
    }
} // namespace SparkyStudios::Audio::Amplitude::Tests
