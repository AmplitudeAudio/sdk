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
#include <Core/RoomInternalState.h>
#include <Mixer/Nodes/ReverbNode.h>

#include "NodeTestCase.h"
#include "TestRegistry.h"
#include "TestUtils.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(NodeTestCase, mixer_nodes, reverb_node_tail_survives_reset)
    {
    public:
        void Run() override
        {
            constexpr AmUInt64 frames = 512;

            auto node = amshared(ReverbNode, "Freeverb");
            auto instance = node->CreateInstance();
            instance->Initialize(1, GetLayer(), nullptr, 0);
            instance->Configure(frames, 2);
            auto* processor = AsProcessor(instance);
            AM_EXPECT_NOT(processor == nullptr);

            RoomInternalState roomState;
            roomState.SetId(10);
            roomState.SetDimensions({ 12.0f, 6.0f, 8.0f });
            fplutil::intrusive_list roomList(&RoomInternalState::node);
            roomList.push_back(roomState);
            roomState.Update();
            Room room(&roomState);

            ChannelInternalState channelState;
            channelState.SetRoomGain(room.GetId(), 1.0f);
            Channel channel(&channelState);

            GetMockLayer().SetRoom(room);
            GetMockLayer().SetChannel(channel);

            AudioBuffer impulse(frames, 1);
            impulse[0][0] = 1.0f;
            AudioBuffer silence(frames, 1);

            // The mixer calls Reset() before every block; the tail must survive it.
            AmReal32 tailEnergy = 0.0f;
            for (int block = 0; block < 8; ++block)
            {
                instance->Reset();
                const AudioBuffer* out = processor->Process(block == 0 ? &impulse : &silence);
                AM_EXPECT_NOT(out == nullptr);

                if (block >= 2)
                    for (AmUInt64 f = 0; f < frames; ++f)
                        tailEnergy += (*out)[0][f] * (*out)[0][f] + (*out)[1][f] * (*out)[1][f];
            }

            AM_EXPECT(tailEnergy > 1e-6f);

            roomList.clear();
        }
    };

    AM_REGISTER_TEST(mixer_nodes, reverb_node_tail_survives_reset);

    AM_TEST_CASE(NodeTestCase, mixer_nodes, reverb_node_mutes_when_room_leaves)
    {
    public:
        void Run() override
        {
            constexpr AmUInt64 frames = 512;

            auto node = amshared(ReverbNode, "Freeverb");
            auto instance = node->CreateInstance();
            instance->Initialize(1, GetLayer(), nullptr, 0);
            instance->Configure(frames, 2);
            auto* processor = AsProcessor(instance);
            AM_EXPECT_NOT(processor == nullptr);

            RoomInternalState roomState;
            roomState.SetId(10);
            roomState.SetDimensions({ 12.0f, 6.0f, 8.0f });
            fplutil::intrusive_list roomList(&RoomInternalState::node);
            roomList.push_back(roomState);
            roomState.Update();
            Room room(&roomState);

            ChannelInternalState channelState;
            channelState.SetRoomGain(room.GetId(), 1.0f);
            Channel channel(&channelState);

            GetMockLayer().SetRoom(room);
            GetMockLayer().SetChannel(channel);

            AudioBuffer impulse(frames, 1);
            impulse[0][0] = 1.0f;
            AudioBuffer silence(frames, 1);

            // Excite the reverb.
            instance->Reset();
            AM_EXPECT_NOT(processor->Process(&impulse) == nullptr);

            // The room leaves: the node skips processing.
            channelState.SetRoomGain(room.GetId(), 0.0f);
            instance->Reset();
            AM_EXPECT(processor->Process(&silence) == nullptr);

            // The room comes back with silent input: the old tail must not replay.
            channelState.SetRoomGain(room.GetId(), 1.0f);
            instance->Reset();
            const AudioBuffer* out = processor->Process(&silence);
            AM_EXPECT_NOT(out == nullptr);

            AmReal64 energy = 0.0;
            for (AmUInt64 f = 0; f < frames; ++f)
                energy += (*out)[0][f] * (*out)[0][f] + (*out)[1][f] * (*out)[1][f];

            AM_EXPECT(energy < 1e-9);

            GetMockLayer().SetRoom(Room());
            GetMockLayer().SetChannel(Channel());
            roomList.clear();
        }
    };

    AM_REGISTER_TEST(mixer_nodes, reverb_node_mutes_when_room_leaves);
} // namespace SparkyStudios::Audio::Amplitude::Tests
