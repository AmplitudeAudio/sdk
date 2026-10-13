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
    // The loop block of a sound definition is optional: a sound without one must play and survive a demotion to a
    // virtual channel (which anchors a cursor from the loop count) without reading a missing table.
    AM_TEST_CASE(EngineTestCase, core_engine, sound_without_loop_block_can_be_stolen_and_played)
    {
    public:
        void Run() override
        {
            SoundHandle noLoop = amEngine->GetSoundHandle("test_sound_08");
            AM_EXPECT(noLoop != nullptr);
            if (noLoop == nullptr)
                return;

            AM_EXPECT_NOT(noLoop->IsLoop());

            StolenScenario scenario;
            const bool ready = StartStolenScenario(
                scenario, amEngine->GetSoundHandle("test_sound_06"), noLoop,
                [](AmUInt64 frames)
                {
                    amEngine->WaitUntilFrames(frames);
                });

            AM_EXPECT(ready);
        }
    };

    AM_REGISTER_TEST(core_engine, sound_without_loop_block_can_be_stolen_and_played);
} // namespace SparkyStudios::Audio::Amplitude::Tests
