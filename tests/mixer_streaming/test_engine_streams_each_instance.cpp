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
#include <Core/Playback/ChannelInstanceInternalState.h>
#include <Sound/Sound.h>

#include "RecordingCodec.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    // Every instance of a streamed, separate-mode channel decodes through a stream of its own: it keeps its own cursor,
    // advances at the playback rate, and nothing ever decodes on the audio thread. A removed instance gives its
    // decoder back while the channel keeps playing.
    AM_TEST_CASE(StreamingEngineTestCase, mixer_streaming, engine_streams_each_instance)
    {
    public:
        void Run() override
        {
            auto* sound = static_cast<SoundImpl*>(amEngine->GetSoundHandle("AMB_Forest"));
            AM_EXPECT(sound != nullptr);
            const AmUInt32 sourceRate = sound->GetFormat().GetSampleRate();

            auto& mixer = amEngine->GetState()->mixer;
            const AmReal64 outputRate = mixer.GetDeviceDescription().mRequestedOutputSampleRate;

            EventCount began;
            Channel channel = amEngine->Play(sound);
            AM_EXPECT(channel.Valid());
            channel.On(eChannelEvent_Begin, &CountEvent, &began);

            AM_EXPECT(FramesUntil(
                [&]()
                {
                    return began.count.load() > 0;
                }));

            channel.EnableInstancing(eChannelInstanceMode_Separate);

            AmReal32 x = 10.0f;
            ChannelInstance a = channel.AddInstance({ x, 0.0f, 0.0f });
            ChannelInstance b;

            // Each publish (forced by moving the instances) drains the audio thread's cursor write-backs.
            const auto drain = [&]()
            {
                x += 0.001f;
                a.SetLocation({ x, 0.0f, 0.0f });
                if (b.Valid())
                    b.SetLocation({ -x, 0.0f, 0.0f });
            };

            // The first instance renders through its own stream.
            AM_EXPECT(FramesUntil(
                [&]()
                {
                    drain();
                    return a.GetState()->GetCursor() > 0;
                }));

            // The second one joins well after: its cursor stays behind.
            for (AmUInt32 i = 0; i < 45; ++i)
                Frame();

            b = channel.AddInstance({ -x, 0.0f, 0.0f });
            AM_EXPECT(FramesUntil(
                [&]()
                {
                    drain();
                    return b.GetState()->GetCursor() > 0;
                }));

            // The voice, then one decoder per instance.
            AM_EXPECT_EQ(3ULL, static_cast<AmUInt64>(RecordingState::Get().GetDecoders().size()));

            for (AmUInt32 i = 0; i < 30; ++i)
            {
                drain();
                Frame();
            }

            drain();
            const AmUInt64 clockBefore = mixer.GetAudioClock();
            const AmUInt64 cursorA1 = a.GetState()->GetCursor();
            const AmUInt64 cursorB1 = b.GetState()->GetCursor();

            for (AmUInt32 i = 0; i < 60; ++i)
            {
                drain();
                Frame();
            }

            drain();
            const AmUInt64 clockAfter = mixer.GetAudioClock();
            const AmUInt64 cursorA2 = a.GetState()->GetCursor();
            const AmUInt64 cursorB2 = b.GetState()->GetCursor();

            // Their cursors are apart by the time b joined (half a second at least).
            AM_EXPECT(cursorA2 > cursorB2);
            AM_EXPECT(cursorA2 - cursorB2 >= sourceRate / 2);

            // Each advances at the playback rate (the source frames per audio-clock frame).
            const AmReal64 expected = static_cast<AmReal64>(clockAfter - clockBefore) * sourceRate / outputRate;
            AM_EXPECT(expected > 0.0);
            AM_EXPECT(static_cast<AmReal64>(cursorA2 - cursorA1) > expected * 0.85);
            AM_EXPECT(static_cast<AmReal64>(cursorA2 - cursorA1) < expected * 1.15);
            AM_EXPECT(static_cast<AmReal64>(cursorB2 - cursorB1) > expected * 0.85);
            AM_EXPECT(static_cast<AmReal64>(cursorB2 - cursorB1) < expected * 1.15);

            AM_EXPECT(AudibleBlocks().load() > 0);
            AM_EXPECT_EQ(0ULL, GetUnderruns());

            // Decoders are only called from the streaming worker.
            const AmUInt64 audioThread = AudioThreadId().load();
            AM_EXPECT(audioThread != 0);
            AM_EXPECT(RecordingState::Get().GetStreamThreads().count(audioThread) == 0);
            AM_EXPECT(RecordingState::Get().GetSeekThreads().count(audioThread) == 0);

            // A removed instance gives its decoder back once the audio thread let go of it, while the channel plays on.
            const auto decoders = RecordingState::Get().GetDecoders();
            AM_EXPECT_EQ(3ULL, static_cast<AmUInt64>(decoders.size()));

            channel.RemoveInstance(b.GetId());
            AM_EXPECT(FramesUntil(
                [&]()
                {
                    drain();
                    const auto state = mixer.GetStreamDiagnosticsForTesting();
                    return state.instanceStreams == 1 && state.retiringInstanceStreams == 0;
                }));

            if (decoders.size() == 3)
            {
                AM_EXPECT(FramesUntil(
                    [&]()
                    {
                        return decoders[2]->closed.load() == 1;
                    }));
                AM_EXPECT_EQ(0, decoders[0]->closed.load());
                AM_EXPECT_EQ(0, decoders[1]->closed.load());
            }

            AM_EXPECT(channel.Playing());
            AM_EXPECT_EQ(0ULL, GetUnderruns());

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

    AM_REGISTER_TEST_DESKTOP_ONLY(mixer_streaming, engine_streams_each_instance);
} // namespace SparkyStudios::Audio::Amplitude::Tests
