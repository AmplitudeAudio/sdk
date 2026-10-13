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
    namespace
    {
        AmSize MixerPipelineCount(const Channel& channel)
        {
            const RealChannel& realChannel = channel.GetState()->GetRealChannel();
            const AmplimixImpl& mixer = amEngine->GetState()->mixer;

            AmSize count = 0;
            for (const AmUInt32 layerId : realChannel.GetMixerLayerIds())
                count += mixer.GetInstancePipelineCount(realChannel.GetID(), layerId);

            return count;
        }

        // A pause lands on the audio thread one block after the frame that flushes it, then fades out over the transport
        // de-click: wait for the voices to report it instead of a number of frames.
        bool VoicesPaused(const Channel& channel)
        {
            const RealChannel& realChannel = channel.GetState()->GetRealChannel();
            const AmplimixImpl& mixer = amEngine->GetState()->mixer;

            for (const AmUInt32 layerId : realChannel.GetMixerLayerIds())
                if (mixer.GetVoiceState(realChannel.GetID(), layerId) != eVoiceState::Paused)
                    return false;

            return true;
        }
    } // namespace

    // With a cap of 1, instance A resamples through its own converter while instance B overflows onto the layer's
    // converter, which follows the layer's rate (pitch x play speed). Both are added while paused and must then advance
    // at the same rate. Play speed drives the rate here because the engine recomputes the pitch every frame.
    class InstanceConverterRateTestCase : public EngineTestCase
    {
    protected:
        // settleFrames: how long the speed set before the instances exist is left to settle (the mixer eases toward
        // it every block, then stops updating the converters). laterSpeed: a speed set once the instances play, or 0.
        void RunWithSpeed(AmReal32 initialSpeed, AmUInt64 settleFrames, AmReal32 laterSpeed)
        {
            AmplimixImpl& mixer = amEngine->GetState()->mixer;
            const AmSize previousCap = mixer.GetMaxInstancePipelines();
            mixer.SetMaxInstancePipelines(1);

            SoundHandle sound = amEngine->GetSoundHandle("test_sound_01");
            Channel channel = amEngine->Play(sound);
            amEngine->WaitUntilFrames(2);
            AM_EXPECT(channel.Valid());

            channel.EnableInstancing(eChannelInstanceMode_Separate);
            RealChannel& realChannel = channel.GetState()->GetRealChannel();
            realChannel.SetSpeed(initialSpeed);
            amEngine->WaitUntilFrames(settleFrames);
            AM_EXPECT(channel.Playing());

            channel.Pause();
            AM_EXPECT(WaitUntil(
                [&]()
                {
                    return VoicesPaused(channel);
                }));

            ChannelInstance a = channel.AddInstance({ 10.0f, 0.0f, 0.0f });
            ChannelInstance b = channel.AddInstance({ -10.0f, 0.0f, 0.0f });
            AM_EXPECT(WaitUntil([&]() { return MixerPipelineCount(channel) == 1; }));

            channel.Resume();

            if (laterSpeed > 0.0f)
            {
                amEngine->WaitUntilFrames(5);
                realChannel.SetSpeed(laterSpeed);
            }

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
                    return a.GetState()->GetCursor() > 20000;
                }));

            channel.Pause();
            AM_EXPECT(WaitUntil(
                [&]()
                {
                    return VoicesPaused(channel);
                }));
            drain();

            const AmUInt64 cursorA = a.GetState()->GetCursor();
            const AmUInt64 cursorB = b.GetState()->GetCursor();

            // The layer converter's history from before the pause may shift B by a frame or two, nothing more.
            const AmUInt64 drift = cursorA > cursorB ? cursorA - cursorB : cursorB - cursorA;
            AM_EXPECT(drift <= 4);

            channel.Stop(0.0);
            amEngine->WaitUntilFrames(2);
            mixer.SetMaxInstancePipelines(previousCap);
        }
    };

    AM_TEST_CASE(InstanceConverterRateTestCase, core_channel_instances, separate_mode_instance_converters_start_at_current_rate)
    {
    public:
        void Run() override
        {
            RunWithSpeed(0.5f, 30, 0.0f);
        }
    };

    AM_REGISTER_TEST(core_channel_instances, separate_mode_instance_converters_start_at_current_rate);

    AM_TEST_CASE(InstanceConverterRateTestCase, core_channel_instances, separate_mode_instance_converters_follow_rate_changes)
    {
    public:
        void Run() override
        {
            RunWithSpeed(1.0f, 2, 1.25f);
        }
    };

    AM_REGISTER_TEST(core_channel_instances, separate_mode_instance_converters_follow_rate_changes);
} // namespace SparkyStudios::Audio::Amplitude::Tests
