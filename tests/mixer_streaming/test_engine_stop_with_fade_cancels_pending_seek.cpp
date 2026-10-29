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
    // A stop with a fade issued while a seek's ring is still filling drops that seek: its stream is never posted to the
    // voice, so no swap happens after the stop, and every decoder is closed once.
    AM_TEST_CASE(StreamingEngineTestCase, mixer_streaming, engine_stop_with_fade_cancels_pending_seek)
    {
    public:
        void Run() override
        {
            auto* sound = static_cast<SoundImpl*>(amEngine->GetSoundHandle("AMB_Forest"));
            AM_EXPECT(sound != nullptr);

            const SoundFormat& format = sound->GetFormat();
            const AmTime lengthMs = static_cast<AmTime>(format.GetFramesCount()) * kAmSecond / static_cast<AmTime>(format.GetSampleRate());

            EventCount began;
            Channel channel = amEngine->Play(sound);
            AM_EXPECT(channel.Valid());
            channel.On(eChannelEvent_Begin, &CountEvent, &began);

            AM_EXPECT(FramesUntil(
                [&]()
                {
                    return began.count.load() > 0;
                }));

            auto& mixer = amEngine->GetState()->mixer;

            // The seek's ring needs far longer to fill than the stop takes to arrive.
            RecordingState::Get().SetDelay(300);
            AM_EXPECT(channel.SetPlaybackPosition(lengthMs * 0.5));
            AM_EXPECT(mixer.GetStreamDiagnosticsForTesting().hasPending);

            channel.Stop(500);

            // The unposted stream is dropped at once, and nothing is queued behind it.
            const auto stopped = mixer.GetStreamDiagnosticsForTesting();
            AM_EXPECT(!stopped.hasPending);
            AM_EXPECT(!stopped.hasQueuedSeek);

            // A seek after the stop is refused.
            AM_EXPECT(!channel.SetPlaybackPosition(lengthMs * 0.25));

            for (AmUInt32 i = 0; i < 60; ++i)
                Frame();

            RecordingState::Get().SetDelay(0);

            AM_EXPECT(FramesUntil(
                []()
                {
                    return RecordingState::Get().AllClosed();
                }));

            for (AmUInt32 i = 0; i < 10; ++i)
                Frame();

            // The voice's decoder, and the one of the cancelled seek if the worker opened it before the stop retired its
            // stream: no swap was ever posted or adopted.
            const auto decoders = static_cast<AmUInt64>(RecordingState::Get().GetDecoders().size());
            AM_EXPECT(decoders >= 1 && decoders <= 2);
            AM_EXPECT(RecordingState::Get().AllClosedOnce());
            AM_EXPECT(!channel.Playing());
        }
    };

    AM_REGISTER_TEST_DESKTOP_ONLY(mixer_streaming, engine_stop_with_fade_cancels_pending_seek);
} // namespace SparkyStudios::Audio::Amplitude::Tests
