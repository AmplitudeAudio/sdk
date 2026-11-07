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
    // A channel stopped while its first ring, or the ring of a seek, is still filling never begins nor swaps, and every
    // stream it created is freed once.
    AM_TEST_CASE(StreamingEngineTestCase, mixer_streaming, engine_stops_streams_mid_prefill)
    {
    public:
        void Run() override
        {
            auto* sound = static_cast<SoundImpl*>(amEngine->GetSoundHandle("AMB_Forest"));
            AM_EXPECT(sound != nullptr);

            const SoundFormat& format = sound->GetFormat();
            const AmTime lengthMs = static_cast<AmTime>(format.GetFramesCount()) * kAmSecond / static_cast<AmTime>(format.GetSampleRate());

            // A decode takes longer than the frame the channel is stopped in: the first ring is not ready by then.
            RecordingState::Get().SetDelay(150);

            // Stopped in the first frame of the channel (a stop issued before the channel got a real channel is not
            // forwarded to its voice, which is unrelated to streaming), before the first ring is ready.
            EventCount firstBegan;
            {
                Channel channel = amEngine->Play(sound);
                AM_EXPECT(channel.Valid());
                channel.On(eChannelEvent_Begin, &CountEvent, &firstBegan);
                Frame();
                channel.Stop(0);
            }

            for (AmUInt32 i = 0; i < 30; ++i)
                Frame();

            AM_EXPECT_EQ(0, firstBegan.count.load());

            // Seeks, then stops before the seek's ring is ready.
            RecordingState::Get().SetDelay(30);
            EventCount secondBegan;
            Channel channel = amEngine->Play(sound);
            AM_EXPECT(channel.Valid());
            channel.On(eChannelEvent_Begin, &CountEvent, &secondBegan);

            AM_EXPECT(FramesUntil(
                [&]()
                {
                    return secondBegan.count.load() > 0;
                }));

            RecordingState::Get().SetDelay(200);
            AM_EXPECT(channel.SetPlaybackPosition(lengthMs * 0.5));
            channel.Stop(0);

            for (AmUInt32 i = 0; i < 30; ++i)
                Frame();

            RecordingState::Get().SetDelay(0);

            AM_EXPECT(FramesUntil(
                []()
                {
                    return RecordingState::Get().AllClosed();
                }));

            for (AmUInt32 i = 0; i < 10; ++i)
                Frame();

            AM_EXPECT(!RecordingState::Get().GetDecoders().empty());
            AM_EXPECT(RecordingState::Get().AllClosedOnce());
            AM_EXPECT(!channel.Playing());
        }
    };

    AM_REGISTER_TEST_DESKTOP_ONLY(mixer_streaming, engine_stops_streams_mid_prefill);
} // namespace SparkyStudios::Audio::Amplitude::Tests
