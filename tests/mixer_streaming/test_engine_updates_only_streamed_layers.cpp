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
    // The per-frame stream update visits the layers that stream, not every layer of the mixer: a sound in memory never
    // joins them, and a streamed one leaves once its layer is gone.
    AM_TEST_CASE(StreamingEngineTestCase, mixer_streaming, engine_updates_only_streamed_layers)
    {
    public:
        void Run() override
        {
            auto& mixer = amEngine->GetState()->mixer;
            AM_EXPECT_EQ(0U, mixer.GetStreamDiagnosticsForTesting().streamedLayers);

            EventCount memoryBegan;
            Channel memory = amEngine->Play(amEngine->GetSoundHandle("test_sound_06"));
            AM_EXPECT(memory.Valid());
            memory.On(eChannelEvent_Begin, &CountEvent, &memoryBegan);
            AM_EXPECT(FramesUntil(
                [&]()
                {
                    return memoryBegan.count.load() > 0;
                }));
            AM_EXPECT_EQ(0U, mixer.GetStreamDiagnosticsForTesting().streamedLayers);

            EventCount streamBegan;
            Channel streamed = amEngine->Play(amEngine->GetSoundHandle("AMB_Forest"));
            AM_EXPECT(streamed.Valid());
            streamed.On(eChannelEvent_Begin, &CountEvent, &streamBegan);
            AM_EXPECT(FramesUntil(
                [&]()
                {
                    return streamBegan.count.load() > 0;
                }));
            AM_EXPECT_EQ(1U, mixer.GetStreamDiagnosticsForTesting().streamedLayers);

            streamed.Stop(0);
            AM_EXPECT(FramesUntil(
                [&]()
                {
                    return mixer.GetStreamDiagnosticsForTesting().streamedLayers == 0;
                }));

            memory.Stop(0);
            for (AmUInt32 i = 0; i < 10; ++i)
                Frame();

            AM_EXPECT_EQ(0U, mixer.GetStreamDiagnosticsForTesting().streamedLayers);
            AM_EXPECT(RecordingState::Get().AllClosedOnce());
        }
    };

    AM_REGISTER_TEST_DESKTOP_ONLY(mixer_streaming, engine_updates_only_streamed_layers);
} // namespace SparkyStudios::Audio::Amplitude::Tests
