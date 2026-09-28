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
#include <Core/Playback/ChannelInternalState.h>
#include <Mixer/Amplimix.h>
#include <Mixer/RealChannel.h>

#include "EngineTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    namespace
    {
        AmSize AttachedCount(const Channel& channel)
        {
            return channel.GetState()->GetRealChannel().GetAttachedInstancePipelineCount();
        }

        // Mixer-side view; valid once the audio thread has run the pending commands.
        AmSize MixerPipelineCount(const Channel& channel)
        {
            const RealChannel& realChannel = channel.GetState()->GetRealChannel();
            const AmplimixImpl& mixer = amEngine->GetState()->mixer;

            AmSize count = 0;
            for (const AmUInt32 layerId : realChannel.GetMixerLayerIds())
                count += mixer.GetInstancePipelineCount(realChannel.GetID(), layerId);

            return count;
        }

        Channel PlaySeparateModeChannel()
        {
            SoundHandle sound = amEngine->GetSoundHandle("test_sound_01");
            Channel channel = amEngine->Play(sound);
            amEngine->WaitUntilFrames(2);
            channel.EnableInstancing(eChannelInstanceMode_Separate);
            return channel;
        }
    } // namespace

    AM_TEST_CASE(EngineTestCase, core_channel_instances, separate_mode_instances_get_own_pipelines)
    {
    public:
        void Run() override
        {
            Channel channel = PlaySeparateModeChannel();
            AM_EXPECT(channel.Valid());

            ChannelInstance a = channel.AddInstance({ 10.0f, 0.0f, 0.0f });
            ChannelInstance b = channel.AddInstance({ -10.0f, 0.0f, 0.0f });
            ChannelInstance c = channel.AddInstance({ 0.0f, 0.0f, 10.0f });
            AM_EXPECT_EQ(AttachedCount(channel), 3);

            // Let the audio thread run the attach commands and mix through the new pipelines.
            amEngine->WaitUntilFrames(4);
            AM_EXPECT(channel.Playing());
            AM_EXPECT(WaitUntil([&]() { return MixerPipelineCount(channel) == 3; }));

            channel.RemoveInstance(b.GetId());
            AM_EXPECT_EQ(AttachedCount(channel), 2);

            amEngine->WaitUntilFrames(2);
            AM_EXPECT(channel.Playing());
            AM_EXPECT(WaitUntil([&]() { return MixerPipelineCount(channel) == 2; }));

            channel.Stop();
            amEngine->WaitUntilFrames(2);
        }
    };

    AM_REGISTER_TEST(core_channel_instances, separate_mode_instances_get_own_pipelines);

    AM_TEST_CASE(EngineTestCase, core_channel_instances, separate_mode_over_cap_uses_overflow)
    {
    public:
        void Run() override
        {
            AmplimixImpl& mixer = amEngine->GetState()->mixer;
            const AmSize previous = mixer.GetMaxInstancePipelines();
            mixer.SetMaxInstancePipelines(2);

            Channel channel = PlaySeparateModeChannel();
            ChannelInstance a = channel.AddInstance({ 10.0f, 0.0f, 0.0f });
            ChannelInstance b = channel.AddInstance({ -10.0f, 0.0f, 0.0f });
            ChannelInstance overflow = channel.AddInstance({ 0.0f, 0.0f, 10.0f });
            AM_EXPECT_EQ(AttachedCount(channel), 2);

            amEngine->WaitUntilFrames(3);
            AM_EXPECT(channel.Playing());
            AM_EXPECT(WaitUntil([&]() { return MixerPipelineCount(channel) == 2; }));

            // Removing the instance that never got a pipeline leaves the attached ones alone.
            channel.RemoveInstance(overflow.GetId());
            AM_EXPECT_EQ(AttachedCount(channel), 2);

            // Freeing a slot lets a new instance get its own pipeline.
            channel.RemoveInstance(a.GetId());
            AM_EXPECT_EQ(AttachedCount(channel), 1);
            ChannelInstance d = channel.AddInstance({ 5.0f, 0.0f, 5.0f });
            AM_EXPECT_EQ(AttachedCount(channel), 2);

            amEngine->WaitUntilFrames(2);

            channel.Stop();
            amEngine->WaitUntilFrames(2);
            mixer.SetMaxInstancePipelines(previous);
        }
    };

    AM_REGISTER_TEST(core_channel_instances, separate_mode_over_cap_uses_overflow);

    AM_TEST_CASE(EngineTestCase, core_channel_instances, separate_mode_cap_zero_uses_shared_pipeline)
    {
    public:
        void Run() override
        {
            AmplimixImpl& mixer = amEngine->GetState()->mixer;
            const AmSize previous = mixer.GetMaxInstancePipelines();
            mixer.SetMaxInstancePipelines(0);

            Channel channel = PlaySeparateModeChannel();
            channel.AddInstance({ 10.0f, 0.0f, 0.0f });
            channel.AddInstance({ -10.0f, 0.0f, 0.0f });
            AM_EXPECT_EQ(AttachedCount(channel), 0);

            amEngine->WaitUntilFrames(3);
            AM_EXPECT(channel.Playing());
            AM_EXPECT(WaitUntil([&]() { return MixerPipelineCount(channel) == 0; }));

            channel.Stop();
            amEngine->WaitUntilFrames(2);
            mixer.SetMaxInstancePipelines(previous);
        }
    };

    AM_REGISTER_TEST(core_channel_instances, separate_mode_cap_zero_uses_shared_pipeline);

    AM_TEST_CASE(EngineTestCase, core_channel_instances, separate_mode_disable_detaches_pipelines)
    {
    public:
        void Run() override
        {
            Channel channel = PlaySeparateModeChannel();
            channel.AddInstance({ 10.0f, 0.0f, 0.0f });
            channel.AddInstance({ -10.0f, 0.0f, 0.0f });
            AM_EXPECT_EQ(AttachedCount(channel), 2);

            // Disabling instancing drops the per-instance pipelines.
            channel.DisableInstancing();
            AM_EXPECT_EQ(AttachedCount(channel), 0);

            // Blended mode never uses per-instance pipelines.
            channel.EnableInstancing(eChannelInstanceMode_Blended);
            channel.AddInstance({ 0.0f, 0.0f, 10.0f });
            AM_EXPECT_EQ(AttachedCount(channel), 0);
            channel.DisableInstancing();
            AM_EXPECT_EQ(AttachedCount(channel), 0);

            amEngine->WaitUntilFrames(2);
            channel.Stop();
            amEngine->WaitUntilFrames(2);
        }
    };

    AM_REGISTER_TEST(core_channel_instances, separate_mode_disable_detaches_pipelines);
} // namespace SparkyStudios::Audio::Amplitude::Tests
