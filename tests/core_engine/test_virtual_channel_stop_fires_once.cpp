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
    // Stop() on a virtual channel fires Stop exactly once.
    AM_TEST_CASE(EngineTestCase, core_engine, virtual_channel_stop_fires_once)
    {
    public:
        void Run() override
        {
            StolenScenario scenario;
            const bool ready = StartStolenScenario(
                scenario, amEngine->GetSoundHandle("test_sound_03"),
                [](AmUInt64 frames)
                {
                    amEngine->WaitUntilFrames(frames);
                });

            AM_EXPECT(ready);
            if (!ready)
                return;

            scenario.stolen->Stop(0.0);
            AM_EXPECT(scenario.stolen->GetState()->Stopped());

            AM_EXPECT(WaitUntil(
                [&]()
                {
                    return scenario.stolenCounter->stopped.load() > 0;
                }));

            amEngine->WaitUntilFrames(10);
            AM_EXPECT_EQ(1, scenario.stolenCounter->stopped.load());
        }
    };

    AM_REGISTER_TEST(core_engine, virtual_channel_stop_fires_once);
} // namespace SparkyStudios::Audio::Amplitude::Tests
