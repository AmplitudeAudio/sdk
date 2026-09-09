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
    AM_TEST_CASE(EngineTestCase, core_engine, channel_voice_stealing)
    {
    public:
        void Run() override
        {
            SoundHandle sound = amEngine->GetSoundHandle("test_sound_01");
            AM_EXPECT(sound != nullptr);
            auto* soundImpl = static_cast<SoundImpl*>(sound);

            PriorityList playingList(&ChannelInternalState::priority_node);
            FreeList realFreeList(&ChannelInternalState::free_node);
            FreeList virtualFreeList(&ChannelInternalState::free_node);

            // Populate 3 active channels: 1.0, 0.8, 0.5
            ChannelInternalState c10;
            c10.SetSound(soundImpl);
            c10.SetGain(1.0f);

            ChannelInternalState c8;
            c8.SetSound(soundImpl);
            c8.SetGain(0.8f);

            ChannelInternalState c5;
            c5.SetSound(soundImpl);
            c5.SetGain(0.5f);

            playingList.push_back(c10);
            playingList.push_back(c8);
            playingList.push_back(c5);

            // CASE 1: Free lists are empty, incoming sound has lower priority (0.2).
            // insertionPoint == playingList.end().
            // Must return nullptr without stealing or halting any sound.
            auto itLow = FindInsertionPoint(&playingList, 0.2f);
            AM_EXPECT(itLow == playingList.end());

            ChannelInternalState* stolenLow = FindFreeChannelInternalState(
                itLow, &playingList, &realFreeList, &virtualFreeList, false);
            AM_EXPECT(stolenLow == nullptr);
            AM_EXPECT_EQ(playingList.size(), 3);
            AM_EXPECT_EQ(playingList.back().Priority(), 0.5f);

            // CASE 2: Free lists are empty, incoming sound has EQUAL priority to lowest sound (0.5).
            // Incumbent protection: insertionPoint == playingList.end().
            // Must return nullptr without stealing or halting c5.
            auto itEqual = FindInsertionPoint(&playingList, 0.5f);
            AM_EXPECT(itEqual == playingList.end());

            ChannelInternalState* stolenEqual = FindFreeChannelInternalState(
                itEqual, &playingList, &realFreeList, &virtualFreeList, false);
            AM_EXPECT(stolenEqual == nullptr);
            AM_EXPECT_EQ(playingList.size(), 3);
            AM_EXPECT_EQ(playingList.back().Priority(), 0.5f);

            // CASE 3: Free lists are empty, incoming sound has HIGHER priority than lowest sound (0.7 > 0.5).
            // insertionPoint points to c5.
            // Lowest sound (c5) should be evicted and returned as newChannel.
            auto itHigher = FindInsertionPoint(&playingList, 0.7f);
            AM_EXPECT(&*itHigher == &c5);

            ChannelInternalState* stolenHigher = FindFreeChannelInternalState(
                itHigher, &playingList, &realFreeList, &virtualFreeList, false);
            AM_EXPECT(stolenHigher != nullptr);
            AM_EXPECT(stolenHigher == &c5);
            // It was at the back and remains at the back since it takes the 0.7 slot
            AM_EXPECT(&playingList.back() == &c5);

            // Update its gain to reflect new sound priority 0.7
            c5.SetGain(0.7f);
            AM_EXPECT_EQ(c5.Priority(), 0.7f);

            // CASE 4: Incoming sound priority 0.9 > 0.7 (higher than c8 and c5, lower than c10).
            // insertionPoint points to c8.
            // Lowest sound (c5) is evicted and moved in front of c8!
            auto itHighest = FindInsertionPoint(&playingList, 0.9f);
            AM_EXPECT(&*itHighest == &c8);

            ChannelInternalState* stolenRelocated = FindFreeChannelInternalState(
                itHighest, &playingList, &realFreeList, &virtualFreeList, false);
            AM_EXPECT(stolenRelocated == &c5);
            c5.SetGain(0.9f);

            // Check resulting descending order: c10 (1.0), c5 (0.9), c8 (0.8)
            auto iter = playingList.begin();
            AM_EXPECT(&*iter == &c10);
            ++iter;
            AM_EXPECT(&*iter == &c5);
            ++iter;
            AM_EXPECT(&*iter == &c8);

            while (!playingList.empty())
                playingList.pop_front();
        }
    };

    AM_REGISTER_TEST(core_engine, channel_voice_stealing);
} // namespace SparkyStudios::Audio::Amplitude::Tests
