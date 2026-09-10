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
#include <Mixer/Nodes/ReverbNode.h>
#include <Core/RoomInternalState.h>
#include <Core/Playback/ChannelInternalState.h>

#include "NodeTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(NodeTestCase, mixer_nodes, reverb_node_room_adaptation)
    {
    public:
        void Run() override
        {
            auto node = amshared(ReverbNode, "Freeverb");
            auto instance = node->CreateInstance();
            AM_EXPECT_NOT(instance == nullptr);

            instance->Initialize(1, GetLayer(), nullptr, 0);
            instance->Configure(512, 2);
            instance->Reset();

            AudioBuffer in(512, 1);
            in[0][0] = 1.0f;

            auto* processor = AsProcessor(instance);
            AM_EXPECT_NOT(processor == nullptr);

            // 1. When no room is active on layer, Process should early return nullptr
            const AudioBuffer* out = processor->Process(&in);
            AM_EXPECT(out == nullptr);

            // 2. Setup active room and channel with room gain
            RoomInternalState roomState;
            roomState.SetId(10);
            constexpr AmVector3 dimensions = { 12.0f, 6.0f, 8.0f };
            roomState.SetDimensions(dimensions);

            fplutil::intrusive_list roomList(&RoomInternalState::node);
            roomList.push_back(roomState);
            roomState.Update();

            Room room(&roomState);
            AM_EXPECT(room.Valid());
            AM_EXPECT(room.GetVolume() > 0.0f);

            ChannelInternalState channelState;
            channelState.SetRoomGain(room.GetId(), 1.0f);

            Channel channel(&channelState);
            AM_EXPECT(channel.Valid());
            AM_EXPECT(channelState.GetRoomGain(room.GetId()) > 0.0f);

            GetMockLayer().SetRoom(room);
            GetMockLayer().SetChannel(channel);

            // Reset should now adapt room parameters from geometry
            instance->Reset();

            // Process multiple blocks to allow the delay line tail to emerge
            AudioBuffer silence(512, 1);
            AmReal32 totalEnergy = 0.0f;

            for (int b = 0; b < 8; ++b)
            {
                const AudioBuffer* wetOut = processor->Process(b == 0 ? &in : &silence);
                AM_EXPECT_NOT(wetOut == nullptr);
                AM_EXPECT_EQ(wetOut->GetChannelCount(), 2);
                AM_EXPECT_EQ(wetOut->GetFrameCount(), 512);

                for (AmUInt64 f = 0; f < 512; ++f)
                {
                    totalEnergy += (*wetOut)[0][f] * (*wetOut)[0][f];
                    totalEnergy += (*wetOut)[1][f] * (*wetOut)[1][f];
                }
            }

            AM_EXPECT(totalEnergy > 0.0f);
        }
    };

    AM_REGISTER_TEST(mixer_nodes, reverb_node_room_adaptation);
} // namespace SparkyStudios::Audio::Amplitude::Tests
