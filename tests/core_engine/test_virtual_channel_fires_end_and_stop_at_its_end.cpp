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
    // A non-looping sound that runs past its end while virtual (the others loop forever, so no real channel frees up to
    // promote it) still fires End and Stop, once each.
    AM_TEST_CASE(EngineTestCase, core_engine, virtual_channel_fires_end_and_stop_at_its_end)
    {
    public:
        void Run() override
        {
            StolenScenario scenario;
            const bool ready = StartStolenScenario(
                scenario, amEngine->GetSoundHandle("test_sound_06"), amEngine->GetSoundHandle("test_sound_05"),
                [](AmUInt64 frames)
                {
                    amEngine->WaitUntilFrames(frames);
                });

            AM_EXPECT(ready);
            if (!ready)
                return;

            AM_EXPECT_NOT(scenario.stolen->GetState()->IsReal());

            AM_EXPECT(WaitUntil(
                [&]()
                {
                    return scenario.stolenCounter->ended.load() > 0 && scenario.stolenCounter->stopped.load() > 0;
                },
                1500));

            // Let any duplicate show up.
            amEngine->WaitUntilFrames(10);
            AM_EXPECT_EQ(1, scenario.stolenCounter->ended.load());
            AM_EXPECT_EQ(1, scenario.stolenCounter->stopped.load());
        }
    };

    AM_REGISTER_TEST(core_engine, virtual_channel_fires_end_and_stop_at_its_end);
} // namespace SparkyStudios::Audio::Amplitude::Tests
