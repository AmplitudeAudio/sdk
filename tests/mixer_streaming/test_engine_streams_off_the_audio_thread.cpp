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

#include "RecordingCodec.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    // Decoders are only called from the streaming worker: never from the audio thread, and (once the worker runs) never
    // from the game thread either.
    AM_TEST_CASE(StreamingEngineTestCase, mixer_streaming, engine_streams_off_the_audio_thread)
    {
    public:
        void Run() override
        {
            Channel channel = amEngine->Play(amEngine->GetSoundHandle("AMB_Forest"));
            AM_EXPECT(channel.Valid());

            // One second of game frames, then until the voice is mixed (the audio thread id is noted by then).
            for (AmUInt32 i = 0; i < 60; ++i)
                Frame();

            AM_EXPECT(FramesUntil(
                []()
                {
                    return AudioThreadId().load() != 0;
                }));
            AM_EXPECT(channel.Playing());

            const AmUInt64 audioThread = AudioThreadId().load();
            const AmUInt64 gameThread = Thread::GetCurrentThreadId();
            AM_EXPECT(audioThread != gameThread);

            const auto streamThreads = RecordingState::Get().GetStreamThreads();
            AM_EXPECT(!streamThreads.empty());
            AM_EXPECT(streamThreads.count(audioThread) == 0);
            AM_EXPECT(streamThreads.count(gameThread) == 0);

            // A loop rewind and a seek never ask the decoder to seek from the audio thread either.
            AM_EXPECT(RecordingState::Get().GetSeekThreads().count(audioThread) == 0);

            AM_EXPECT_EQ(0ULL, GetUnderruns());

            channel.Stop(0);
        }
    };

    AM_REGISTER_TEST_DESKTOP_ONLY(mixer_streaming, engine_streams_off_the_audio_thread);
} // namespace SparkyStudios::Audio::Amplitude::Tests
