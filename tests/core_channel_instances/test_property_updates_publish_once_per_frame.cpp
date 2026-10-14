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
        constexpr AmSize kInstanceCount = 8;

        const ChannelInstanceData* FindPublished(const Channel& channel, AmChannelInstanceID id)
        {
            for (const auto& data : channel.GetState()->GetPublishedInstanceSnapshot())
                if (data.instanceId == id)
                    return &data;

            return nullptr;
        }

        bool PublishedMatches(const Channel& channel, const std::vector<ChannelInstance>& instances, AmReal32 x, AmReal32 weight)
        {
            for (const auto& instance : instances)
            {
                const ChannelInstanceData* data = FindPublished(channel, instance.GetId());
                if (data == nullptr || std::abs(data->location.x - x) > kEpsilon || std::abs(data->weight - weight) > kEpsilon)
                    return false;
            }

            return true;
        }
    } // namespace

    AM_TEST_CASE(EngineTestCase, core_channel_instances, property_updates_publish_once_per_frame)
    {
    public:
        void Run() override
        {
            SoundHandle test_sound_01 = amEngine->GetSoundHandle("test_sound_01");

            Channel channel = amEngine->Play(test_sound_01);
            amEngine->WaitUntilFrames(2);
            AM_EXPECT(channel.Valid());

            channel.EnableInstancing(eChannelInstanceMode_Blended);

            std::vector<ChannelInstance> instances;
            for (AmSize i = 0; i < kInstanceCount; ++i)
            {
                ChannelInstance instance = channel.AddInstance({ static_cast<AmReal32>(i), 0.0f, 0.0f });
                AM_EXPECT(instance.Valid());
                instances.push_back(instance);
            }

            const AmUInt64 before = channel.GetState()->GetInstanceSnapshotPublishCountForTesting();

            for (const auto& instance : instances)
            {
                instance.SetLocation({ 100.0f, 0.0f, 0.0f });
                instance.SetWeight(0.25f);
            }

            // Eager publishing would have cost one rebuild per setter. A frame boundary may fall inside the loop.
            const AmUInt64 published = channel.GetState()->GetInstanceSnapshotPublishCountForTesting() - before;
            AM_EXPECT(published <= 2);

            // The final values still reach the snapshot once the frame flushes it.
            amEngine->WaitUntilFrames(3);
            AM_EXPECT(PublishedMatches(channel, instances, 100.0f, 0.25f));
        }
    };

    AM_TEST_CASE(EngineTestCase, core_channel_instances, property_updates_reach_the_snapshot_while_paused)
    {
    public:
        void Run() override
        {
            SoundHandle test_sound_01 = amEngine->GetSoundHandle("test_sound_01");

            Channel channel = amEngine->Play(test_sound_01);
            amEngine->WaitUntilFrames(2);
            AM_EXPECT(channel.Valid());

            channel.EnableInstancing(eChannelInstanceMode_Blended);

            std::vector<ChannelInstance> instances;
            instances.push_back(channel.AddInstance({ 1.0f, 0.0f, 0.0f }));
            AM_EXPECT(instances[0].Valid());

            channel.Pause();
            amEngine->WaitUntilFrames(2);

            instances[0].SetLocation({ 42.0f, 0.0f, 0.0f });
            instances[0].SetWeight(0.5f);

            amEngine->WaitUntilFrames(3);
            AM_EXPECT(PublishedMatches(channel, instances, 42.0f, 0.5f));
        }
    };

    AM_REGISTER_TEST(core_channel_instances, property_updates_publish_once_per_frame);
    AM_REGISTER_TEST(core_channel_instances, property_updates_reach_the_snapshot_while_paused);
} // namespace SparkyStudios::Audio::Amplitude::Tests
