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

#include <functional>
#include <vector>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Core/Engine.h>
#include <Core/EngineInternalState.h>
#include <Core/Playback/ChannelInternalState.h>
#include <Mixer/Amplimix.h>
#include <Mixer/RealChannel.h>

#include "EngineTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    /// Runs the steal-and-stop scenario for a kind of channel that has no virtual cursor (collections, switch containers).
    class DemotedContainerTestCase : public EngineTestCase
    {
    protected:
        template<typename Play>
        void CheckDemotedChannelIsKept(Play&& play)
        {
            std::vector<Channel> low;
            std::vector<AmChannelID> realIds;
            for (int i = 0; i < 50; ++i)
                low.push_back(play(i, 0.5f));

            amEngine->WaitUntilFrames(2);
            for (const auto& channel : low)
                realIds.push_back(channel.GetState()->GetRealChannel().GetID());

            // Every voice must have started, or the steal below would release voices that never rendered.
            AM_EXPECT(WaitUntil(
                [&]()
                {
                    for (std::size_t c = 0; c < low.size(); ++c)
                    {
                        const auto ids = low[c].GetState()->GetRealChannel().GetMixerLayerIds();
                        if (ids.empty() || amEngine->GetState()->mixer.GetVoiceState(realIds[c], ids[0]) == eVoiceState::Scheduled)
                            return false;
                    }

                    return true;
                }));

            Channel high = play(50, 1.0f);

            // Catch the steal within a millisecond: the old voices' release is then still running.
            ChannelInternalState* state = nullptr;
            AmChannelID oldId = kAmInvalidObjectId;
            std::vector<AmUInt32> layers;
            for (int i = 0; i < 2000 && state == nullptr; ++i)
            {
                for (std::size_t c = 0; c < low.size(); ++c)
                {
                    if (!low[c].GetState()->IsReal() && !low[c].GetState()->GetRealChannel().GetMixerLayerIds().empty())
                    {
                        state = low[c].GetState();
                        oldId = realIds[c];
                        layers = state->GetRealChannel().GetMixerLayerIds();
                        break;
                    }
                }

                if (state == nullptr)
                    Thread::Sleep(1);
            }

            AM_EXPECT_NOT(state == nullptr);
            if (state == nullptr)
                return;

            AM_EXPECT_NOT(layers.empty());
            if (layers.empty())
                return;

            const AmplimixImpl& mixer = amEngine->GetState()->mixer;

            // Stopped at once, while the release is still running.
            state->Halt();

            bool recycledWhileRendering = false;
            for (int frame = 0; frame < 60; ++frame)
            {
                // The channel is read first: a voice still rendering after that cannot have finished before the channel
                // was recycled. A finished voice (its layer is only destroyed on a later sweep) no longer renders.
                const bool recycled = !state->IsAlive() || !state->GetListener().Valid();
                const eVoiceState voiceState = mixer.GetVoiceState(oldId, layers[0]);
                const bool rendering =
                    voiceState == eVoiceState::Playing || voiceState == eVoiceState::FadingOut || voiceState == eVoiceState::Ending;
                if (recycled && rendering)
                    recycledWhileRendering = true;

                amEngine->WaitUntilFrames(1);
            }

            AM_EXPECT_NOT(recycledWhileRendering);

            // And it is recycled once the voices are gone.
            AM_EXPECT(WaitUntil(
                [&]()
                {
                    return !state->IsAlive();
                }));

            for (auto& channel : low)
                channel.Stop(0.0);
            high.Stop(0.0);
        }
    };

    AM_TEST_CASE(DemotedContainerTestCase, core_engine, demoted_collection_is_not_recycled_while_releasing)
    {
    public:
        void Run() override
        {
            CollectionHandle collection = amEngine->GetCollectionHandle("test_collection");
            CheckDemotedChannelIsKept(
                [&](int, AmReal32 gain)
                {
                    return amEngine->Play(collection, AmVector3{ 0.0f, 0.0f, 0.0f }, gain);
                });
        }
    };

    AM_TEST_CASE(DemotedContainerTestCase, core_engine, demoted_switch_container_is_not_recycled_while_releasing)
    {
    public:
        void Run() override
        {
            SwitchContainerHandle footsteps = amEngine->GetSwitchContainerHandle("footsteps");

            // The container is entity scoped: one entity per channel.
            std::vector<Entity> entities;
            for (int i = 0; i <= 50; ++i)
                entities.push_back(amEngine->AddEntity(200 + i));

            CheckDemotedChannelIsKept(
                [&](int index, AmReal32 gain)
                {
                    return amEngine->Play(footsteps, entities[static_cast<std::size_t>(index)], gain);
                });

            for (int i = 0; i <= 50; ++i)
                amEngine->RemoveEntity(200 + i);
        }
    };

    AM_REGISTER_TEST(core_engine, demoted_collection_is_not_recycled_while_releasing);
    AM_REGISTER_TEST(core_engine, demoted_switch_container_is_not_recycled_while_releasing);
} // namespace SparkyStudios::Audio::Amplitude::Tests
