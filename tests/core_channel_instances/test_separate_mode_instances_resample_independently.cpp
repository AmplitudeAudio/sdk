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

#include <Core/Engine.h>
#include <Core/Playback/ChannelInstanceInternalState.h>
#include <Core/Playback/ChannelInternalState.h>
#include <Mixer/Amplimix.h>
#include <Mixer/RealChannel.h>

#include "EngineTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    // test_sound_01 is 44.1 kHz and the test device runs at 48 kHz, so every instance goes through the resampler.
    AM_TEST_CASE(EngineTestCase, core_channel_instances, separate_mode_instances_resample_independently)
    {
    public:
        void Run() override
        {
            SoundHandle sound = amEngine->GetSoundHandle("test_sound_01");
            Channel channel = amEngine->Play(sound);
            amEngine->WaitUntilFrames(2);
            AM_EXPECT(channel.Valid());

            channel.EnableInstancing(eChannelInstanceMode_Separate);

            // Added while paused, and resumed once both render through their own pipeline and converter: both start
            // at cursor 0 and must advance by the same amount every block.
            channel.Pause();
            amEngine->WaitUntilFrames(5);

            ChannelInstance a = channel.AddInstance({ 10.0f, 0.0f, 0.0f });
            ChannelInstance b = channel.AddInstance({ -10.0f, 0.0f, 0.0f });

            const RealChannel& realChannel = channel.GetState()->GetRealChannel();
            const AmplimixImpl& mixer = amEngine->GetState()->mixer;
            AM_EXPECT(WaitUntil(
                [&]()
                {
                    AmSize count = 0;
                    for (const AmUInt32 layerId : realChannel.GetMixerLayerIds())
                        count += mixer.GetInstancePipelineCount(realChannel.GetID(), layerId);

                    return count == 2;
                }));

            channel.Resume();

            // Each publish (forced by moving an instance) drains the audio thread's cursor write-backs.
            AmReal32 x = 10.0f;
            const auto drain = [&]()
            {
                x += 0.01f;
                a.SetLocation({ x, 0.0f, 0.0f });
            };

            AM_EXPECT(WaitUntil(
                [&]()
                {
                    drain();
                    return a.GetState()->GetCursor() > 10000;
                }));

            // Freeze playback so both cursors are read from the same block.
            channel.Pause();
            amEngine->WaitUntilFrames(5);
            drain();

            const AmUInt64 cursorA = a.GetState()->GetCursor();
            const AmUInt64 cursorB = b.GetState()->GetCursor();

            AM_EXPECT(cursorB > 0);
            AM_EXPECT_EQ(cursorA, cursorB);

            channel.Stop(0.0);
        }
    };

    AM_REGISTER_TEST(core_channel_instances, separate_mode_instances_resample_independently);
} // namespace SparkyStudios::Audio::Amplitude::Tests
