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

#include <Core/Playback/ChannelInstanceInternalState.h>

#include "EngineTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    // Characterization test guarding the Task 4 refactor: separate-mode instance
    // cursors must keep their positions across pause/resume. (The audio thread owns
    // cursors while the layer is alive; the channel state receives them through the
    // write-back slots at publish time.)
    AM_TEST_CASE(EngineTestCase, core_channel_instances, separate_mode_cursors_survive_pause_resume)
    {
    public:
        void Run() override
        {
            SoundHandle test_sound_01 = amEngine->GetSoundHandle("test_sound_01");

            Channel channel = amEngine->Play(test_sound_01);
            amEngine->WaitUntilFrames(2);
            AM_EXPECT(channel.Valid());

            channel.EnableInstancing(eChannelInstanceMode_Separate);
            ChannelInstance instance = channel.AddInstance({ 100.0f, 0.0f, 50.0f });
            AM_EXPECT(instance.Valid());

            // Let playback advance, then force a publish so the write-back cursors
            // are drained into the channel state where the test can observe them.
            amEngine->WaitUntilFrames(6);
            instance.SetLocation({ 101.0f, 0.0f, 50.0f });

            const AmUInt64 cursorBeforePause = instance.GetState()->GetCursor();
            AM_EXPECT(cursorBeforePause > 0);

            channel.Pause();
            amEngine->WaitUntilFrames(3);

            channel.Resume();
            amEngine->WaitUntilFrames(3);

            // Force a publish to drain the cursors again.
            instance.SetLocation({ 102.0f, 0.0f, 50.0f });

            const AmUInt64 cursorAfterResume = instance.GetState()->GetCursor();

            // Continued playback: the cursor must not have restarted from zero.
            AM_EXPECT(cursorAfterResume > cursorBeforePause);
        }
    };

    AM_REGISTER_TEST(core_channel_instances, separate_mode_cursors_survive_pause_resume);
} // namespace SparkyStudios::Audio::Amplitude::Tests
