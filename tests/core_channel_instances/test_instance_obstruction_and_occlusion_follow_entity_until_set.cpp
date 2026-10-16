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

#include <optional>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Core/Playback/ChannelInternalState.h>

#include "EngineTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    namespace
    {
        std::optional<ChannelInstanceData> FindPublished(const Channel& channel, AmChannelInstanceID id)
        {
            for (const auto& data : channel.GetState()->GetPublishedInstanceSnapshot())
                if (data.instanceId == id)
                    return data;

            return std::nullopt;
        }
    } // namespace

    AM_TEST_CASE(EngineTestCase, core_channel_instances, instance_obstruction_and_occlusion_follow_entity_until_set)
    {
    public:
        void Run() override
        {
            // Entity scoped: the channel only follows the entity when the played object has the entity scope.
            CollectionHandle footsteps = amEngine->GetCollectionHandle("grass_footsteps");

            Entity entity = amEngine->AddEntity(4343);
            AM_EXPECT(entity.Valid());
            entity.SetObstruction(0.3f);
            entity.SetOcclusion(0.6f);

            Channel channel = amEngine->Play(footsteps, entity);
            amEngine->WaitUntilFrames(2);
            AM_EXPECT(channel.Valid());

            // Paused: a slow runner must not let the sound end, and take its instances with it, before the test is done.
            channel.Pause();

            channel.EnableInstancing(eChannelInstanceMode_Separate);

            ChannelInstance instance = channel.AddInstance({ 1.0f, 0.0f, 0.0f });
            AM_EXPECT(instance.Valid());

            // Without values of its own, the instance follows the entity's, and the mixer is told so.
            AM_EXPECT_EQ(instance.GetObstruction(), 0.3f);
            AM_EXPECT_EQ(instance.GetOcclusion(), 0.6f);

            auto published = FindPublished(channel, instance.GetId());
            AM_EXPECT(published.has_value());
            AM_EXPECT_NOT(published->hasObstruction);
            AM_EXPECT_NOT(published->hasOcclusion);

            // Values set on the instance win, and are published to the mixer. The entity's no longer applies.
            instance.SetObstruction(0.9f);
            instance.SetOcclusion(0.0f);
            entity.SetObstruction(0.1f);
            entity.SetOcclusion(0.2f);

            AM_EXPECT_EQ(instance.GetObstruction(), 0.9f);
            AM_EXPECT_EQ(instance.GetOcclusion(), 0.0f);

            amEngine->WaitUntilFrames(3);

            published = FindPublished(channel, instance.GetId());
            AM_EXPECT(published.has_value());
            AM_EXPECT(published->hasObstruction && published->obstruction == 0.9f);
            AM_EXPECT(published->hasOcclusion && published->occlusion == 0.0f);

            // Resetting one makes it follow the entity again, independently of the other.
            instance.ResetObstruction();
            AM_EXPECT_EQ(instance.GetObstruction(), 0.1f);
            AM_EXPECT_EQ(instance.GetOcclusion(), 0.0f);

            amEngine->WaitUntilFrames(3);

            published = FindPublished(channel, instance.GetId());
            AM_EXPECT(published.has_value());
            AM_EXPECT_NOT(published->hasObstruction);
            AM_EXPECT(published->hasOcclusion);

            instance.ResetOcclusion();
            AM_EXPECT_EQ(instance.GetOcclusion(), 0.2f);

            channel.Stop(0.0);
            amEngine->RemoveEntity(4343);
        }
    };

    AM_REGISTER_TEST(core_channel_instances, instance_obstruction_and_occlusion_follow_entity_until_set);
} // namespace SparkyStudios::Audio::Amplitude::Tests
