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

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    // The engine frame walks the instances of an entity-scoped channel in separate mode to assign their rooms, while
    // the game thread adds and removes them. The walk must hold the instance lock, or it follows a node that was
    // just unlinked and freed.
    AM_TEST_CASE(EngineTestCase, core_channel_instances, instance_churn_with_room_tracking_does_not_crash)
    {
    public:
        void Run() override
        {
            CollectionHandle footsteps = amEngine->GetCollectionHandle("grass_footsteps");

            Entity entity = amEngine->AddEntity(4343);
            AM_EXPECT(entity.Valid());

            Channel channel = amEngine->Play(footsteps, entity);
            amEngine->WaitUntilFrames(2);
            AM_EXPECT(channel.Valid());

            channel.EnableInstancing(eChannelInstanceMode_Separate);

            for (AmUInt32 round = 0; round < 1500; ++round)
            {
                std::vector<ChannelInstance> instances;
                for (AmUInt32 i = 0; i < 4; ++i)
                    instances.push_back(channel.AddInstance({ static_cast<AmReal32>(round % 50), static_cast<AmReal32>(i), 0.0f }));

                for (const auto& instance : instances)
                    if (instance.Valid())
                        channel.RemoveInstance(instance.GetId());
            }

            AM_EXPECT_EQ(channel.GetInstanceCount(), 0u);
            AM_EXPECT(channel.Valid());

            channel.DisableInstancing();
            channel.Stop(0.0);
            amEngine->RemoveEntity(4343);
        }
    };

    AM_REGISTER_TEST(core_channel_instances, instance_churn_with_room_tracking_does_not_crash);
} // namespace SparkyStudios::Audio::Amplitude::Tests
