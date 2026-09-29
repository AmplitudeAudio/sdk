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
#include <Core/Playback/ChannelInternalState.h>
#include <Mixer/Amplimix.h>
#include <Mixer/RealChannel.h>

#include "EngineTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(EngineTestCase, core_engine, stale_finished_event_does_not_touch_reused_channel)
    {
    public:
        void Run() override
        {
            SoundHandle sound = amEngine->GetSoundHandle("test_sound_01");

            // Stop a voice and immediately start another: the free list hands the stopped channel state to the new
            // sound while the old voice is still de-clicking. The old voice's Finished must release only its own layer.
            for (int round = 0; round < 8; ++round)
            {
                Channel first = amEngine->Play(sound);
                AM_EXPECT(WaitUntil(
                    [&]()
                    {
                        return first.Playing();
                    }));
                ChannelInternalState* firstState = first.GetState();

                first.Stop(0.0);
                Channel second = amEngine->Play(sound);
                AM_EXPECT(WaitUntil(
                    [&]()
                    {
                        return second.Playing();
                    }));

                // Long enough for the first voice's Finished to be dispatched.
                amEngine->WaitUntilFrames(10);
                AM_EXPECT(second.Playing());
                AM_EXPECT_EQ(1ULL, second.GetState()->GetRealChannel().GetMixerLayerIds().size());

                if (second.GetState() == firstState)
                    amLogDebug("Round %d reused the channel state.", round);

                second.Stop(0.0);
                AM_EXPECT(WaitUntil(
                    [&]()
                    {
                        return !second.Playing();
                    }));
            }
        }
    };

    AM_REGISTER_TEST(core_engine, stale_finished_event_does_not_touch_reused_channel);
} // namespace SparkyStudios::Audio::Amplitude::Tests
