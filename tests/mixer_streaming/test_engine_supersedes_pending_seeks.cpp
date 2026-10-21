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
    // Seeks made while a stream is still filling supersede each other, and the one made while a swap is in flight waits
    // for it: playback ends up at the last target, and every decoder opened for the seeks is closed exactly once.
    AM_TEST_CASE(StreamingEngineTestCase, mixer_streaming, engine_supersedes_pending_seeks)
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
                    return began.count.load() > 0;
                }));

            // From now on every decode takes 30 ms: a seek's ring needs several frames to fill.
            RecordingState::Get().SetDelay(30);

            auto& mixer = amEngine->GetState()->mixer;

            const AmTime first = lengthMs * 0.8;
            const AmTime second = lengthMs * 0.6;
            const AmTime third = lengthMs * 0.2;
            const AmTime last = lengthMs * 0.4;

            // Three seeks in one frame: the first two never reach the voice, each superseded by the next.
            AM_EXPECT(channel.SetPlaybackPosition(first));
            AM_EXPECT(channel.SetPlaybackPosition(second));
            AM_EXPECT(channel.SetPlaybackPosition(third));
            AM_EXPECT(!mixer.GetStreamDiagnosticsForTesting().swapPosted);

            // Wait until the third seek is posted to the voice, so the next one is requested while a swap is in flight.
            AM_EXPECT(FramesUntil(
                [&]()
                {
                    return mixer.GetStreamDiagnosticsForTesting().swapPosted;
                },
                150));

            AM_EXPECT(channel.SetPlaybackPosition(last));
            AM_EXPECT(mixer.GetStreamDiagnosticsForTesting().hasQueuedSeek);

            // Playback ends up within a second of the last target.
            const AmTime window = 1000.0;
            AM_EXPECT(FramesUntil(
                [&]()
                {
                    const AmTime position = channel.GetPlaybackPosition();
                    return position >= last && position < last + window;
                },
                150));

            // Every posted seek was resolved once, nothing is left pending or queued.
            AM_EXPECT(FramesUntil(
                [&]()
                {
                    const auto state = mixer.GetStreamDiagnosticsForTesting();
                    return !state.swapPosted && !state.hasPending && !state.hasQueuedSeek;
                },
                150));

            const auto state = mixer.GetStreamDiagnosticsForTesting();
            AM_EXPECT_EQ(2U, state.swapsPosted);
            AM_EXPECT_EQ(2U, state.swapsAdopted);
            AM_EXPECT_EQ(0U, state.swapsDropped);

            // The voice's decoder, then one per seek: the two superseded ones were freed before the end of the test.
            const auto decoders = RecordingState::Get().GetDecoders();
            AM_EXPECT_EQ(5ULL, static_cast<AmUInt64>(decoders.size()));
            if (decoders.size() == 5)
            {
                AM_EXPECT(FramesUntil(
                    [&]()
                    {
                        return decoders[1]->closed.load() == 1 && decoders[2]->closed.load() == 1;
                    },
                    150));
            }

            RecordingState::Get().SetDelay(0);
            channel.Stop(0);

            AM_EXPECT(FramesUntil(
                []()
                {
                    return RecordingState::Get().AllClosed();
                }));

            // Let late frees run, then nothing was closed twice.
            for (AmUInt32 i = 0; i < 10; ++i)
                Frame();

            AM_EXPECT(RecordingState::Get().AllClosedOnce());
        }
    };

    AM_REGISTER_TEST_DESKTOP_ONLY(mixer_streaming, engine_supersedes_pending_seeks);
} // namespace SparkyStudios::Audio::Amplitude::Tests
