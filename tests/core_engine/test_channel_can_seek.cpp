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

#include "EngineTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(EngineTestCase, core_engine, channel_can_seek)
    {
    public:
        void Run() override
        {
            SoundHandle sound = amEngine->GetSoundHandle("test_sound_01");

            Channel channel = amEngine->Play(sound);
            amEngine->WaitUntilFrames(2);

            AM_EXPECT(channel.Valid());
            AM_EXPECT(channel.Playing());

            constexpr AmTime seekPosition = 1000.0;
            AM_EXPECT(channel.GetPlaybackPosition() < seekPosition);
            AM_EXPECT(channel.SetPlaybackPosition(seekPosition));
            amEngine->WaitUntilFrames(4);

            const AmTime playbackPosition = channel.GetPlaybackPosition();
            AM_EXPECT(playbackPosition >= seekPosition);

            channel.Stop(0);
        }
    };

    AM_REGISTER_TEST(core_engine, channel_can_seek);
} // namespace SparkyStudios::Audio::Amplitude::Tests
