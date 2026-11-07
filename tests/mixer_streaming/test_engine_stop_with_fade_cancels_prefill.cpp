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

#include <Sound/Sound.h>

#include "RecordingCodec.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    // A stop with a fade issued while the first ring is still filling cancels the start: nothing is ever heard, the
    // channel never begins, and the decoder is closed once.
    AM_TEST_CASE(StreamingEngineTestCase, mixer_streaming, engine_stop_with_fade_cancels_prefill)
    {
    public:
        void Run() override
        {
            auto* sound = static_cast<SoundImpl*>(amEngine->GetSoundHandle("AMB_Forest"));
            AM_EXPECT(sound != nullptr);

            // The first ring is not ready by the time the channel is stopped.
            RecordingState::Get().SetDelay(150);

            EventCount began;
            Channel channel = amEngine->Play(sound);
            AM_EXPECT(channel.Valid());
            channel.On(eChannelEvent_Begin, &CountEvent, &began);

            Frame();
            channel.Stop(500);

            // Well past the fade and the decodes: nothing was rendered, nothing began.
            for (AmUInt32 i = 0; i < 60; ++i)
                Frame();

            AM_EXPECT_EQ(0, began.count.load());
            AM_EXPECT_EQ(0U, AudibleBlocks().load());

            RecordingState::Get().SetDelay(0);

            AM_EXPECT(FramesUntil(
                []()
                {
                    return RecordingState::Get().AllClosed();
                }));

            for (AmUInt32 i = 0; i < 10; ++i)
                Frame();

            AM_EXPECT_EQ(1ULL, static_cast<AmUInt64>(RecordingState::Get().GetDecoders().size()));
            AM_EXPECT(RecordingState::Get().AllClosedOnce());
            AM_EXPECT_EQ(0, began.count.load());
            AM_EXPECT(!channel.Playing());
        }
    };

    AM_REGISTER_TEST_DESKTOP_ONLY(mixer_streaming, engine_stop_with_fade_cancels_prefill);
} // namespace SparkyStudios::Audio::Amplitude::Tests
