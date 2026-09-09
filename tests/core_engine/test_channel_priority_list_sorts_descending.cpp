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
    AM_TEST_CASE(EngineTestCase, core_engine, channel_priority_list_sorts_descending)
    {
    public:
        void Run() override
        {
            SoundHandle sound = amEngine->GetSoundHandle("test_sound_01");
            AM_EXPECT(sound != nullptr);
            auto* soundImpl = static_cast<SoundImpl*>(sound);

            PriorityList list(&ChannelInternalState::priority_node);

            ChannelInternalState c1;
            c1.SetSound(soundImpl);
            c1.SetGain(0.5f); // priority: 0.5 * 1.0 = 0.5

            ChannelInternalState c2;
            c2.SetSound(soundImpl);
            c2.SetGain(1.0f); // priority: 1.0 * 1.0 = 1.0

            ChannelInternalState c3;
            c3.SetSound(soundImpl);
            c3.SetGain(0.8f); // priority: 0.8 * 1.0 = 0.8

            ChannelInternalState c4;
            c4.SetSound(soundImpl);
            c4.SetGain(0.2f); // priority: 0.2 * 1.0 = 0.2

            ChannelInternalState c5;
            c5.SetSound(soundImpl);
            c5.SetGain(1.2f); // priority: 1.2 * 1.0 = 1.2

            list.push_back(c1);
            list.push_back(c2);
            list.push_back(c3);
            list.push_back(c4);
            list.push_back(c5);

            list.sort(ChannelPriorityComparator{});

            // Verify strictly descending order
            std::vector<AmReal32> expected = { 1.2f, 1.0f, 0.8f, 0.5f, 0.2f };
            size_t idx = 0;
            for (const auto& item : list)
            {
                AM_EXPECT_EQ(item.Priority(), expected[idx]);
                ++idx;
            }
            AM_EXPECT_EQ(idx, expected.size());

            // Clean up list nodes before stack destruction
            while (!list.empty())
                list.pop_front();
        }
    };
    AM_REGISTER_TEST(core_engine, channel_priority_list_sorts_descending);
} // namespace SparkyStudios::Audio::Amplitude::Tests
