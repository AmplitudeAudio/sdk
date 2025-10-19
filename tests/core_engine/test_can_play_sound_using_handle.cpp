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
        SoundHandle test_sound_01 = amEngine->GetSoundHandle("test_sound_01");

        Channel channel = amEngine->Play(test_sound_01);
        amEngine->WaitUntilFrames(2); // Playing is done in the next frame

        AM_EXPECT(channel.Valid());
        AM_EXPECT(channel.Playing());

        Thread::Sleep(kAmSecond * 5); // wait for the sound to finish playing
        AM_EXPECT_NOT(channel.Playing());

        channel.Stop(0);
    }
} // namespace SparkyStudios::Audio::Amplitude::Tests
