// Copyright (c) 2021-present Sparky Studios. All rights reserved.
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

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    // Entity states are pooled. Removing an entity that still has a sound playing must detach that sound,
    // otherwise the next entity to be handed the same recycled slot inherits a voice that is not its own.
    AM_TEST_CASE(EngineTestCase, core_engine, removed_entity_does_not_leak_sounds_to_recycled_slot)
    {
    public:
        void Run() override
        {
            // Entity scoped: this collection only attaches to the channel when played with an entity.
            CollectionHandle footsteps = amEngine->GetCollectionHandle("grass_footsteps");
            AM_EXPECT_NOT(footsteps == nullptr);

            Entity e1 = amEngine->AddEntity(1);
            AM_EXPECT(e1.Valid());

            Channel channel = amEngine->Play(footsteps, e1);
            amEngine->WaitUntilFrames(2);
            AM_EXPECT(channel.Valid());

            // Paused: keeps the voice (and its attachment to the entity) alive for the rest of the test.
            channel.Pause();

            AM_EXPECT_EQ(e1.GetActiveChannelCount(), 1);

            // Remove the entity while its sound is still attached and playing.
            amEngine->RemoveEntity(1);

            // The free list is LIFO, so the very next AddEntity() is handed back this same recycled state.
            Entity e2 = amEngine->AddEntity(2);
            AM_EXPECT(e2.Valid());
            AM_EXPECT_EQ(e2.GetState(), e1.GetState());

            AM_EXPECT_EQ(e2.GetActiveChannelCount(), 0);

            channel.Stop(0.0);
            amEngine->RemoveEntity(2);
        }
    };

    AM_REGISTER_TEST(core_engine, removed_entity_does_not_leak_sounds_to_recycled_slot);
} // namespace SparkyStudios::Audio::Amplitude::Tests
