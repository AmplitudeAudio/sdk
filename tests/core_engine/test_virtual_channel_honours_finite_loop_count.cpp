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

#include <deque>
#include <vector>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Sound/Sound.h>

#include "EngineTestCase.h"
#include "TestRegistry.h"
#include "VirtualChannelTestUtils.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    // A sound looping a finite number of times ends on the game side at the right time while its channel is virtual
    // (test_sound_04 plays test_sound_03 three times).
    AM_TEST_CASE(EngineTestCase, core_engine, virtual_channel_honours_finite_loop_count)
    {
    public:
        void Run() override
        {
            SoundHandle finite = amEngine->GetSoundHandle("test_sound_04");
            const SoundFormat& format = static_cast<SoundImpl*>(finite)->GetFormat();
            const AmReal64 passSeconds = static_cast<AmReal64>(format.GetFramesCount()) / format.GetSampleRate();
            const AmplimixImpl& mixer = amEngine->GetState()->mixer;
            const AmReal64 outputRate = mixer.GetDeviceDescription().mRequestedOutputSampleRate;

            const AmUInt64 startClock = mixer.GetAudioClock();

            // The others loop forever, so no real channel frees up: the tail one stays virtual.
            StolenScenario scenario;
            const bool ready = StartStolenScenario(
                scenario, amEngine->GetSoundHandle("test_sound_06"), finite,
                [](AmUInt64 frames)
                {
                    amEngine->WaitUntilFrames(frames);
                });

            AM_EXPECT(ready);
            if (!ready)
                return;

            EventCounter& counter = *scenario.stolenCounter;

            AM_EXPECT(WaitUntil(
                [&]()
                {
                    return counter.ended.load() > 0;
                },
                1500));

            amEngine->WaitUntilFrames(10);
            AM_EXPECT_EQ(1, counter.ended.load());

            // Three passes in total, give or take the steal fade and scheduling.
            const AmReal64 elapsed = static_cast<AmReal64>(counter.endClock.load() - startClock) / outputRate;
            AM_EXPECT(elapsed > 3.0 * passSeconds - 0.4);
            AM_EXPECT(elapsed < 3.0 * passSeconds + 0.6);

            for (auto& channel : scenario.low)
                channel.Stop(0.0);
            scenario.high.Stop(0.0);
        }
    };

    AM_REGISTER_TEST(core_engine, virtual_channel_honours_finite_loop_count);
} // namespace SparkyStudios::Audio::Amplitude::Tests
