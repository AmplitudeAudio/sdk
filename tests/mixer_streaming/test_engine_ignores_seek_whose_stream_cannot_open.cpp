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
    // A seek opens its decoder on the streaming worker. When that open fails, the seek is given up: the sound plays on
    // from where it was instead of ending on an empty stream, and the failed decoder is still closed.
    AM_TEST_CASE(StreamingEngineTestCase, mixer_streaming, engine_ignores_seek_whose_stream_cannot_open)
    {
    public:
        void Run() override
        {
            auto* sound = static_cast<SoundImpl*>(amEngine->GetSoundHandle("AMB_Forest"));
            AM_EXPECT(sound != nullptr);

            const SoundFormat& format = sound->GetFormat();
            const AmTime lengthMs = static_cast<AmTime>(format.GetFramesCount()) * kAmSecond / static_cast<AmTime>(format.GetSampleRate());
            AM_EXPECT(lengthMs > 10000.0);

            EventCount began;
            Channel channel = amEngine->Play(sound);
            AM_EXPECT(channel.Valid());
            channel.On(eChannelEvent_Begin, &CountEvent, &began);

            AM_EXPECT(FramesUntil(
                [&]()
                {
                    return began.count.load() > 0 && channel.GetPlaybackPosition() > 100.0;
                }));

            auto& mixer = amEngine->GetState()->mixer;
            const AmTime before = channel.GetPlaybackPosition();

            RecordingState::Get().SetFailOpens(true);
            AM_EXPECT(channel.SetPlaybackPosition(lengthMs * 0.8));

            AM_EXPECT(FramesUntil(
                [&]()
                {
                    return !mixer.GetStreamDiagnosticsForTesting().hasPending;
                }));

            RecordingState::Get().SetFailOpens(false);

            const auto state = mixer.GetStreamDiagnosticsForTesting();
            AM_EXPECT_EQ(0U, state.swapsPosted);
            AM_EXPECT(!state.swapPosted);

            // It plays on from where it was, far from the seek target.
            AM_EXPECT(FramesUntil(
                [&]()
                {
                    return channel.GetPlaybackPosition() > before + 200.0;
                }));
            AM_EXPECT(channel.Playing());
            AM_EXPECT(channel.GetPlaybackPosition() < lengthMs * 0.5);

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

    AM_REGISTER_TEST_DESKTOP_ONLY(mixer_streaming, engine_ignores_seek_whose_stream_cannot_open);
} // namespace SparkyStudios::Audio::Amplitude::Tests
