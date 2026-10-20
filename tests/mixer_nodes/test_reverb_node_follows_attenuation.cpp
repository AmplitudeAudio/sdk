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
#include "TestUtils.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(NodeTestCase, mixer_nodes, reverb_node_follows_attenuation)
    {
    public:
        void Run() override
        {
            constexpr AmUInt64 frameCount = 512;
            constexpr AmReal64 maxDistance = 100.0;

            RoomInternalState roomState;
            roomState.SetId(10);
            roomState.SetDimensions({ 12.0f, 6.0f, 8.0f });

            fplutil::intrusive_list roomList(&RoomInternalState::node);
            roomList.push_back(roomState);
            roomState.Update();

            Room room(&roomState);

            // The source is inside the room, so its room gain is at its maximum whatever the listener does.
            ChannelInternalState channelState;
            channelState.SetRoomGain(room.GetId(), 1.0f);
            Channel channel(&channelState);

            ListenerInternalState listenerState;
            listenerState.SetLocation({ 0.0f, 0.0f, 0.0f });
            listenerState.SetOrientation(Orientation({ 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f }));
            listenerState.Update();
            Listener listener = InitTestListener(&listenerState);

            MockAttenuation attenuation(maxDistance);

            GetMockLayer().SetSpatialization(eSpatialization_Position);
            GetMockLayer().SetListener(listener);
            GetMockLayer().SetAttenuation(&attenuation);
            GetMockLayer().SetRoom(room);
            GetMockLayer().SetChannel(channel);

            AudioBuffer input(frameCount, 1);
            input[0][0] = 1.0f;

            const auto energyOf = [&](const AmVector3& location) -> AmReal32
            {
                GetMockLayer().SetLocation(location);

                auto node = amshared(ReverbNode, "Freeverb");
                auto instance = node->CreateInstance();
                instance->Initialize(1, GetLayer(), nullptr, 0);
                instance->Configure(frameCount, 2);

                auto* processor = AsProcessor(instance);
                AM_EXPECT_NOT(processor == nullptr);

                AudioBuffer silence(frameCount, 1);
                AmReal32 energy = 0.0f;

                for (int b = 0; b < 8; ++b)
                {
                    const AudioBuffer* out = processor->Process(b == 0 ? &input : &silence);
                    if (out == nullptr)
                        continue;

                    for (AmUInt64 f = 0; f < frameCount; ++f)
                        energy += (*out)[0][f] * (*out)[0][f] + (*out)[1][f] * (*out)[1][f];
                }

                return energy;
            };

            // Within the attenuation range, the room send is heard.
            AM_EXPECT(energyOf({ 0.0f, 0.0f, -5.0f }) > 0.0f);

            // Past the max distance, the attenuation silences the sound, and so it must the room send.
            AM_EXPECT_EQ(energyOf({ 0.0f, 0.0f, -(static_cast<AmReal32>(maxDistance) + 50.0f) }), 0.0f);
        }
    };

    AM_REGISTER_TEST(mixer_nodes, reverb_node_follows_attenuation);
} // namespace SparkyStudios::Audio::Amplitude::Tests
