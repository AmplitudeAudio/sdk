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

#include <vector>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include "EngineTestCase.h"
#include "TestRegistry.h"
#include "VirtualChannelTestUtils.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    // A paused channel that loses its real channel stays virtual and paused (no silent stop, no Stop event) until the game
    // resumes it; it then plays on and stops exactly once.
    AM_TEST_CASE(EngineTestCase, core_engine, channel_stolen_while_paused_stays_paused)
    {
    public:
        void Run() override
        {
            SoundHandle sound = amEngine->GetSoundHandle("test_sound_06");

            std::vector<Channel> low;
            EventCounter* counter = NewCounter();
            for (int i = 0; i < 50; ++i)
            {
                // The last one is the quietest: the one a louder sound takes a real channel from.
                low.push_back(amEngine->Play(sound, AmVector3{ 0.0f, 0.0f, 0.0f }, i == 49 ? 0.4f : 0.5f));
                if (i == 49)
                    low.back().On(eChannelEvent_Stop, &CountStop, counter);
            }

            amEngine->WaitUntilFrames(2);

            Channel& paused = low.back();
            AM_EXPECT(paused.GetState()->IsReal());
            paused.Pause(0.0);
            AM_EXPECT(paused.GetState()->Paused());

            Channel high = amEngine->Play(sound, AmVector3{ 0.0f, 0.0f, 0.0f }, 1.0f);
            AM_EXPECT(WaitUntil(
                [&]()
                {
                    return !paused.GetState()->IsReal();
                }));

            // Virtual and still paused, for as long as nobody resumes it.
            amEngine->WaitUntilFrames(20);
            AM_EXPECT(paused.GetState()->IsAlive());
            AM_EXPECT_NOT(paused.GetState()->IsReal());
            AM_EXPECT(paused.GetState()->Paused());
            AM_EXPECT_EQ(0, counter->stopped.load());

            // Resumed while virtual: it plays on (virtually) instead of stopping.
            paused.Resume(0.0);
            AM_EXPECT(paused.GetState()->Playing());
            amEngine->WaitUntilFrames(20);
            AM_EXPECT(paused.GetState()->Playing());
            AM_EXPECT_EQ(0, counter->stopped.load());

            paused.Stop(0.0);
            AM_EXPECT(WaitUntil(
                [&]()
                {
                    return counter->stopped.load() > 0;
                }));

            amEngine->WaitUntilFrames(10);
            AM_EXPECT_EQ(1, counter->stopped.load());

            for (auto& channel : low)
                channel.Stop(0.0);
            high.Stop(0.0);
        }
    };

    AM_REGISTER_TEST(core_engine, channel_stolen_while_paused_stays_paused);
} // namespace SparkyStudios::Audio::Amplitude::Tests
