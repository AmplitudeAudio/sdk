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

#include <Core/EngineInternalState.h>
#include <Sound/Sound.h>

#include "RecordingCodec.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    // An instance removed while its stream is still filling is never attached: its stream is retired once, its decoder
    // is closed, and no instance decoder is left open or created again by later updates.
    AM_TEST_CASE(StreamingEngineTestCase, mixer_streaming, engine_detaches_pending_instance_stream)
    {
    public:
        void Run() override
        {
            auto& mixer = amEngine->GetState()->mixer;

            EventCount began;
            Channel channel = amEngine->Play(amEngine->GetSoundHandle("AMB_Forest"));
            AM_EXPECT(channel.Valid());
            channel.On(eChannelEvent_Begin, &CountEvent, &began);

            AM_EXPECT(FramesUntil(
                [&]()
                {
                    return began.count.load() > 0;
                }));

            channel.EnableInstancing(eChannelInstanceMode_Separate);

            // Every decode takes a while: the instance's prefill is still pending when it is removed.
            RecordingState::Get().SetDelay(250);

            ChannelInstance instance = channel.AddInstance({ 5.0f, 0.0f, 0.0f });
            AM_EXPECT(FramesUntil(
                [&]()
                {
                    return mixer.GetStreamDiagnosticsForTesting().pendingInstanceAttaches == 1;
                },
                5));

            // The attach is recorded: updates while it waits never open another decoder.
            for (AmUInt32 i = 0; i < 5; ++i)
                Frame();

            const auto decoders = RecordingState::Get().GetDecoders();
            AM_EXPECT_EQ(2ULL, static_cast<AmUInt64>(decoders.size()));
            AM_EXPECT_EQ(1U, mixer.GetStreamDiagnosticsForTesting().pendingInstanceAttaches);

            channel.RemoveInstance(instance.GetId());

            {
                const auto state = mixer.GetStreamDiagnosticsForTesting();
                AM_EXPECT_EQ(0U, state.pendingInstanceAttaches);
                AM_EXPECT_EQ(0U, state.instanceStreams);
                AM_EXPECT_EQ(0U, state.retiringInstanceStreams);
            }

            // Its decoder is closed (once) while the channel keeps playing.
            if (decoders.size() == 2)
            {
                AM_EXPECT(FramesUntil(
                    [&]()
                    {
                        return decoders[1]->closed.load() == 1;
                    }));
                AM_EXPECT_EQ(0, decoders[0]->closed.load());
            }

            RecordingState::Get().SetDelay(0);

            // Later updates do not bring it back.
            for (AmUInt32 i = 0; i < 20; ++i)
                Frame();

            AM_EXPECT_EQ(2ULL, static_cast<AmUInt64>(RecordingState::Get().GetDecoders().size()));
            AM_EXPECT_EQ(0U, mixer.GetStreamDiagnosticsForTesting().pendingInstanceAttaches);
            AM_EXPECT(channel.Playing());

            channel.Stop(0);

            AM_EXPECT(FramesUntil(
                []()
                {
                    return RecordingState::Get().AllClosed();
                }));

            for (AmUInt32 i = 0; i < 10; ++i)
                Frame();

            AM_EXPECT(RecordingState::Get().AllClosedOnce());
        }
    };

    AM_REGISTER_TEST_DESKTOP_ONLY(mixer_streaming, engine_detaches_pending_instance_stream);
} // namespace SparkyStudios::Audio::Amplitude::Tests
