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
#include <Sound/Sound.h>

#include "EngineTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    // The game thread builds per-instance pipelines outside the audio thread's control, reading the layer's sound. A layer
    // pinned for that must not be destroyed by the audio thread until it is unpinned.
    AM_TEST_CASE(EngineTestCase, core_channel_instances, pinned_layer_defers_destroy)
    {
    public:
        void Run() override
        {
            SoundHandle handle = amEngine->GetSoundHandle("test_sound_01");
            const auto* sound = static_cast<SoundImpl*>(handle);
            const AmInt32 idleRefCount = sound->GetRefCounter()->GetCount();
            const auto soundRefCount = [sound]()
            {
                return sound->GetRefCounter()->GetCount();
            };

            Channel channel = amEngine->Play(handle);
            AM_EXPECT(WaitUntil([&]() { return channel.Playing(); }));
            const AmInt32 playingRefCount = soundRefCount();
            AM_EXPECT(playingRefCount > idleRefCount);

            const RealChannel& realChannel = channel.GetState()->GetRealChannel();
            const std::vector<AmUInt32> layerIds = realChannel.GetMixerLayerIds();
            AM_EXPECT_EQ(layerIds.size(), 1);

            AmplimixImpl& mixer = amEngine->GetState()->mixer;
            const AmUInt32 id = realChannel.GetID();
            const AmUInt32 layer = layerIds.front();

            AM_EXPECT(mixer.PinLayer(id, layer));

            channel.Stop(0.0);
            AM_EXPECT(WaitUntil([&]() { return !channel.Playing(); }));

            // Several mix cycles: the destroy is requested but must wait for the pin.
            amEngine->WaitUntilFrames(30);
            AM_EXPECT_EQ(soundRefCount(), playingRefCount);

            mixer.UnpinLayer(layer);
            AM_EXPECT(WaitUntil([&]() { return soundRefCount() == idleRefCount; }));
        }
    };

    AM_REGISTER_TEST(core_channel_instances, pinned_layer_defers_destroy);
} // namespace SparkyStudios::Audio::Amplitude::Tests
