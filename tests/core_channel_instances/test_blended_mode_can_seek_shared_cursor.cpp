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
    AM_TEST_CASE(EngineTestCase, core_channel_instances, blended_mode_can_seek_shared_cursor)
    {
    public:
        void Run() override
        {
            // A looping sound: a one-shot can reach its end on a slow runner before the seek is observed.
            SoundHandle sound = amEngine->GetSoundHandle("test_sound_07");

            Channel channel = amEngine->Play(sound);
            amEngine->WaitUntilFrames(2);

            AM_EXPECT(channel.Valid());
            if (!channel.Valid())
                return;

            channel.EnableInstancing(eChannelInstanceMode_Blended);
            AM_UNUSED(channel.AddInstance({ 100.0f, 0.0f, 50.0f }));
            AM_UNUSED(channel.AddInstance({ 120.0f, 0.0f, 80.0f }));

            AM_EXPECT(channel.GetInstanceCount() == 2);
            AM_EXPECT(channel.GetInstancingMode() == eChannelInstanceMode_Blended);

            constexpr AmTime seekPosition = 500.0;
            AM_EXPECT(channel.GetPlaybackPosition() < seekPosition);
            AM_EXPECT(channel.SetPlaybackPosition(seekPosition));

            // The seek is applied by a mixer command on the audio thread. The sound loops, so the channel stays alive
            // until the seek is observed; a channel that is gone fails the test instead of passing it.
            AmTime playbackPosition = 0.0;
            AM_EXPECT(WaitUntil(
                [&]()
                {
                    if (!channel.Valid())
                        return true;

                    playbackPosition = channel.GetPlaybackPosition();
                    return playbackPosition >= seekPosition;
                }));

            AM_EXPECT(channel.Valid());
            AM_EXPECT(playbackPosition >= seekPosition);

            if (channel.Valid())
                channel.Stop(0);
        }
    };

    AM_REGISTER_TEST(core_channel_instances, blended_mode_can_seek_shared_cursor);
} // namespace SparkyStudios::Audio::Amplitude::Tests
