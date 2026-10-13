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

#include "EngineTestCase.h"
#include "TestRegistry.h"
#include "VirtualChannelTestUtils.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    // A channel whose real channel is taken while it fades out to a stop settles Stopped with one Stop event: it is
    // not kept alive as a virtual channel (a looping sound would then never stop).
    AM_TEST_CASE(EngineTestCase, core_engine, channel_stolen_during_stop_fade_settles_stopped)
    {
    public:
        void Run() override
        {
            // The others loop forever; the tail one loops three times (~4 s): kept virtual, it would keep going for seconds.
            SoundHandle sound = amEngine->GetSoundHandle("test_sound_06");
            SoundHandle finite = amEngine->GetSoundHandle("test_sound_04");

            std::vector<EventCounter*> counters;
            std::vector<Channel> low;
            for (int i = 0; i < 50; ++i)
            {
                counters.push_back(NewCounter());
                low.push_back(amEngine->Play(i == 49 ? finite : sound, AmVector3{ 0.0f, 0.0f, 0.0f }, 0.5f));
                low.back().On(eChannelEvent_Stop, &CountStop, counters.back());
            }

            amEngine->WaitUntilFrames(2);

            // The last one played is the lowest in the priority list: the one a louder sound takes a channel from.
            Channel& fading = low.back();
            EventCounter& counter = *counters.back();
            AM_EXPECT(fading.GetState()->IsReal());
            fading.Stop(200.0);

            Channel high = amEngine->Play(sound, AmVector3{ 0.0f, 0.0f, 0.0f }, 1.0f);

            // Stopped long before the 200 ms fade or the sound could end, and no longer real.
            AM_EXPECT(WaitUntil(
                [&]()
                {
                    return fading.GetState()->Stopped();
                },
                15));
            AM_EXPECT_NOT(fading.GetState()->IsReal());

            AM_EXPECT(WaitUntil(
                [&]()
                {
                    return counter.stopped.load() > 0;
                }));
            amEngine->WaitUntilFrames(20);
            AM_EXPECT_EQ(1, counter.stopped.load());

            for (auto& channel : low)
                channel.Stop(0.0);
            high.Stop(0.0);
        }
    };

    AM_REGISTER_TEST(core_engine, channel_stolen_during_stop_fade_settles_stopped);
} // namespace SparkyStudios::Audio::Amplitude::Tests
