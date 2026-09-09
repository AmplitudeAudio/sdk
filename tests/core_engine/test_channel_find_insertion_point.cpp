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
#include <Core/Playback/ChannelInternalState.h>
#include <Sound/Sound.h>

#include "EngineTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(EngineTestCase, core_engine, channel_find_insertion_point)
    {
    public:
        void Run() override
        {
            SoundHandle sound = amEngine->GetSoundHandle("test_sound_01");
            AM_EXPECT(sound != nullptr);
            auto* soundImpl = static_cast<SoundImpl*>(sound);

            PriorityList list(&ChannelInternalState::priority_node);

            ChannelInternalState c10;
            c10.SetSound(soundImpl);
            c10.SetGain(1.0f); // priority 1.0

            ChannelInternalState c8;
            c8.SetSound(soundImpl);
            c8.SetGain(0.8f); // priority 0.8

            ChannelInternalState c5;
            c5.SetSound(soundImpl);
            c5.SetGain(0.5f); // priority 0.5

            list.push_back(c10);
            list.push_back(c8);
            list.push_back(c5);

            // 1. Higher than all existing sounds (1.2 > 1.0):
            // Should return list.begin() so insert_before puts it at the front.
            auto itHigh = FindInsertionPoint(&list, 1.2f);
            AM_EXPECT(itHigh == list.begin());

            // 2. Intermediate priority (0.7 is between 0.8 and 0.5):
            // Should return iterator pointing to c5 so insert_before puts it before c5.
            auto itMid = FindInsertionPoint(&list, 0.7f);
            AM_EXPECT(&*itMid == &c5);

            // 3. Equal to an incumbent (0.8 == c8):
            // Incumbent protection: new 0.8 should be placed AFTER c8, so insert_before c5.
            auto itEqualIncumbent = FindInsertionPoint(&list, 0.8f);
            AM_EXPECT(&*itEqualIncumbent == &c5);

            // 4. Equal to lowest sound (0.5 == c5):
            // Incumbent protection: new 0.5 should be placed AFTER c5, returning list.end().
            auto itEqualLowest = FindInsertionPoint(&list, 0.5f);
            AM_EXPECT(itEqualLowest == list.end());

            // 5. Strictly lower than all sounds (0.2 < 0.5):
            // Should return list.end().
            auto itLow = FindInsertionPoint(&list, 0.2f);
            AM_EXPECT(itLow == list.end());

            while (!list.empty())
                list.pop_front();
        }
    };

    AM_REGISTER_TEST(core_engine, channel_find_insertion_point);
} // namespace SparkyStudios::Audio::Amplitude::Tests
