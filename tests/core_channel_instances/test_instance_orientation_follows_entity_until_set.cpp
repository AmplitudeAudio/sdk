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

#include <Core/Playback/ChannelInternalState.h>

#include "EngineTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    namespace
    {
        bool SameForward(const Orientation& a, const Orientation& b)
        {
            return std::abs(a.GetForward().x - b.GetForward().x) < 1e-4f && std::abs(a.GetForward().y - b.GetForward().y) < 1e-4f &&
                std::abs(a.GetForward().z - b.GetForward().z) < 1e-4f;
        }

        const ChannelInstanceData* FindPublished(const Channel& channel, AmChannelInstanceID id)
        {
            for (const auto& data : channel.GetState()->GetPublishedInstanceSnapshot())
                if (data.instanceId == id)
                    return &data;

            return nullptr;
        }
    } // namespace

    AM_TEST_CASE(EngineTestCase, core_channel_instances, instance_orientation_follows_entity_until_set)
    {
    public:
        void Run() override
        {
            // Entity scoped: the channel only follows the entity when the played object has the entity scope.
            CollectionHandle footsteps = amEngine->GetCollectionHandle("grass_footsteps");

            Entity entity = amEngine->AddEntity(4242);
            AM_EXPECT(entity.Valid());

            const Orientation entityOrientation(0.8f, 0.0f, 0.0f);
            entity.SetOrientation(entityOrientation);

            Channel channel = amEngine->Play(footsteps, entity);
            amEngine->WaitUntilFrames(2);
            AM_EXPECT(channel.Valid());
            AM_EXPECT(channel.GetState()->GetEntity().Valid());

            channel.EnableInstancing(eChannelInstanceMode_Blended);

            ChannelInstance instance = channel.AddInstance({ 1.0f, 0.0f, 0.0f });
            AM_EXPECT(instance.Valid());

            // Without an orientation of its own, the instance follows the entity's, and the mixer is told so.
            AM_EXPECT(SameForward(instance.GetOrientation(), entityOrientation));

            const ChannelInstanceData* published = FindPublished(channel, instance.GetId());
            AM_EXPECT_NOT(published == nullptr);
            AM_EXPECT_NOT(published != nullptr && published->hasOrientation);

            // An orientation set on the instance wins, and is published to the mixer.
            const Orientation instanceOrientation(-0.5f, 0.0f, 0.0f);
            instance.SetOrientation(instanceOrientation);

            AM_EXPECT(SameForward(instance.GetOrientation(), instanceOrientation));
            AM_EXPECT(std::abs(instance.GetDirection().x - instanceOrientation.GetForward().x) < 1e-4f);
            AM_EXPECT(std::abs(instance.GetUp().y - instanceOrientation.GetUp().y) < 1e-4f);

            amEngine->WaitUntilFrames(3);

            published = FindPublished(channel, instance.GetId());
            AM_EXPECT_NOT(published == nullptr);
            AM_EXPECT(published != nullptr && published->hasOrientation);
            AM_EXPECT(published != nullptr && SameForward(published->orientation, instanceOrientation));

            // Resetting makes it follow the entity again.
            instance.ResetOrientation();
            AM_EXPECT(SameForward(instance.GetOrientation(), entityOrientation));

            amEngine->WaitUntilFrames(3);

            published = FindPublished(channel, instance.GetId());
            AM_EXPECT_NOT(published == nullptr);
            AM_EXPECT_NOT(published != nullptr && published->hasOrientation);

            channel.Stop(0.0);
            amEngine->RemoveEntity(4242);
        }
    };

    AM_REGISTER_TEST(core_channel_instances, instance_orientation_follows_entity_until_set);
} // namespace SparkyStudios::Audio::Amplitude::Tests
