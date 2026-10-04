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

#include <Core/Engine.h>
#include <Core/EngineInternalState.h>

#include "EngineTestCase.h"
#include "TestRegistry.h"
#include "VirtualChannelTestUtils.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    // A demoted channel stopped right away must not be recycled while its released voice still renders: Reset()
    // clears the listener that voice reads through its channel (the AmbisonicRotatorNode reads it through
    // Listener::GetOrientation()). Runs on the real BinauralHighQuality pipeline.
    AM_TEST_CASE(EngineTestCase, core_engine, demoted_channel_is_not_recycled_while_releasing)
    {
    public:
        void Run() override
        {
            SoundHandle sound = amEngine->GetSoundHandle("test_sound_01");

            std::vector<Channel> low;
            std::vector<AmChannelID> realIds;
            for (int i = 0; i < 50; ++i)
                low.push_back(amEngine->Play(sound, AmVector3{ 0.0f, 0.0f, 0.0f }, 0.5f));

            amEngine->WaitUntilFrames(2);
            for (const auto& channel : low)
                realIds.push_back(channel.GetState()->GetRealChannel().GetID());

            Channel high = amEngine->Play(sound, AmVector3{ 0.0f, 0.0f, 0.0f }, 1.0f);

            // Catch the steal within a millisecond: the old voice's 10 ms release is then still running.
            ChannelInternalState* state = nullptr;
            AmChannelID oldId = kAmInvalidObjectId;
            for (int i = 0; i < 2000 && state == nullptr; ++i)
            {
                for (std::size_t c = 0; c < low.size(); ++c)
                {
                    if (!low[c].GetState()->IsReal())
                    {
                        state = low[c].GetState();
                        oldId = realIds[c];
                        break;
                    }
                }

                if (state == nullptr)
                    Thread::Sleep(1);
            }

            AM_EXPECT_NOT(state == nullptr);
            if (state == nullptr)
                return;

            const std::vector<AmUInt32> layers = state->GetRealChannel().GetMixerLayerIds();
            AM_EXPECT_EQ(std::size_t{ 1 }, layers.size());
            if (layers.empty())
                return;

            const AmplimixImpl& mixer = amEngine->GetState()->mixer;
            AM_EXPECT(mixer.GetVoiceState(oldId, layers[0]) != eVoiceState::Idle);

            // Stopped at once, while that release is still running.
            state->Halt();
            AM_EXPECT(state->Stopped());

            bool recycledWhileRendering = false;
            for (int frame = 0; frame < 60; ++frame)
            {
                // While the released voice renders, the channel is kept and keeps the listener it reads; Reset() would
                // clear it (the AmbisonicRotatorNode reads it through Listener::GetOrientation()). The channel is read
                // first: a voice still rendering after that cannot have finished before the channel was recycled. A
                // finished voice no longer renders, even while its layer waits for a later sweep.
                const bool recycled = !state->IsAlive() || !state->GetListener().Valid();
                const eVoiceState voiceState = mixer.GetVoiceState(oldId, layers[0]);
                const bool rendering =
                    voiceState == eVoiceState::Playing || voiceState == eVoiceState::FadingOut || voiceState == eVoiceState::Ending;
                if (recycled && rendering)
                    recycledWhileRendering = true;

                amEngine->WaitUntilFrames(1);
            }

            AM_EXPECT_NOT(recycledWhileRendering);

            // And it is recycled once the voice is gone.
            AM_EXPECT(WaitUntil(
                [&]()
                {
                    return !state->IsAlive();
                }));
        }
    };

    AM_REGISTER_TEST(core_engine, demoted_channel_is_not_recycled_while_releasing);
} // namespace SparkyStudios::Audio::Amplitude::Tests
