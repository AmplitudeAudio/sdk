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

#include <Ambisonics/AmbisonicComponent.h>
#include <Core/ListenerInternalState.h>
#include <Core/Playback/ChannelInternalState.h>
#include <Core/RoomInternalState.h>
#include <Mixer/Nodes/ReflectionsNode.h>

#include "LightEngineTestCase.h"
#include "NodeTestCase.h"
#include "TestRegistry.h"
#include "TestUtils.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(LightEngineTestCase, mixer_nodes, reflections_arrive_from_the_nearest_wall)
    {
    public:
        void SetUp() override
        {
            LightEngineTestCase::SetUp();
            InitializeConfig(AM_OS_STRING("tests.config.amconfig"));
        }

        void Run() override
        {
            // The ambisonic field has X forward, Y right and Z up (the panning node's ForHRTF encode), so a world
            // direction (x, y, z) in the engine frame reads (X, Y, Z) = (y, x, z). The node outputs a world-frame field.
            const Orientation unrotated = Orientation::Zero();
            CheckNearestWall(unrotated, { -4.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f }); // left wall
            CheckNearestWall(unrotated, { 4.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }); // right wall
            CheckNearestWall(unrotated, { 0.0f, 4.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }); // front wall
            CheckNearestWall(unrotated, { 0.0f, -4.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }); // back wall
            CheckNearestWall(unrotated, { 0.0f, 0.0f, -4.0f }, { 0.0f, 0.0f, -1.0f }); // floor
            CheckNearestWall(unrotated, { 0.0f, 0.0f, 4.0f }, { 0.0f, 0.0f, 1.0f }); // ceiling

            // Room yawed 90 degrees left: its front wall faces world -X, its right wall world +Y.
            const Orientation yawed({ -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });
            CheckNearestWall(yawed, { -4.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f });
            CheckNearestWall(yawed, { 0.0f, 4.0f, 0.0f }, { 1.0f, 0.0f, 0.0f });

            // Room pitched 90 degrees up: its front wall faces world +Z, its floor world +Y.
            const Orientation pitched({ 0.0f, 0.0f, 1.0f }, { 0.0f, -1.0f, 0.0f });
            CheckNearestWall(pitched, { 0.0f, 0.0f, 4.0f }, { 0.0f, 0.0f, 1.0f });
            CheckNearestWall(pitched, { 0.0f, 4.0f, 0.0f }, { 1.0f, 0.0f, 0.0f });
        }

    private:
        // Plays an impulse in a 10 m cube centred on the origin, and checks the direction of the first reflection to
        // arrive, which comes from the wall nearest to the listener.
        void CheckNearestWall(const Orientation& roomOrientation, const AmVector3& listenerLocation, const AmVector3& expected)
        {
            RoomInternalState roomState;
            roomState.SetId(10);
            roomState.SetOrientation(roomOrientation);
            for (AmSize wall = 0; wall < kAmRoomSurfaceCount; ++wall)
                roomState.SetWallMaterial(static_cast<eRoomWall>(wall), RoomWallMaterial(eRoomWallMaterialType_BrickPainted));
            roomState.SetDimensions({ 10.0f, 10.0f, 10.0f });

            fplutil::intrusive_list roomList(&RoomInternalState::node);
            roomList.push_back(roomState);
            roomState.Update();
            Room room(&roomState);

            ChannelInternalState channelState;
            channelState.SetRoomGain(room.GetId(), 1.0f);
            Channel channel(&channelState);

            ListenerInternalState listenerState;
            listenerState.SetLocation(listenerLocation);
            listenerState.SetOrientation(Orientation::Zero());
            listenerState.Update();
            Listener listener = InitTestListener(&listenerState);

            SpatialMockLayer mockLayer;
            mockLayer.SetSampleRate(48000);
            mockLayer.SetSpatialization(eSpatialization_HRTF);
            mockLayer.SetListener(listener);
            mockLayer.SetRoom(room);
            mockLayer.SetChannel(channel);

            const AmUInt64 frameCount = amEngine->GetSamplesPerStream();

            ReflectionsNode factory;
            auto instance = factory.CreateInstance();
            instance->Initialize(1, &mockLayer, nullptr, factory.GetParameterCount());
            instance->Configure(frameCount, 1);
            auto* processor = dynamic_cast<ProcessorNodeInstance*>(instance.get());

            AudioBuffer impulse(frameCount, 1);
            impulse[0][0] = 1.0f;
            AudioBuffer silence(frameCount, 1);

            bool found = false;
            for (int block = 0; block < 8 && !found; ++block)
            {
                const AudioBuffer* field = processor->Process(block == 0 ? &impulse : &silence);
                AM_EXPECT_NOT(field == nullptr);

                for (AmUInt64 i = 0; i < frameCount && !found; ++i)
                {
                    const AmReal32 w = (*field)[eBFormatChannel_W][i];
                    if (std::abs(w) < 1e-6f)
                        continue;

                    found = true;
                    AM_EXPECT(std::abs((*field)[eBFormatChannel_X][i] / w - expected.x) < 1e-3f);
                    AM_EXPECT(std::abs((*field)[eBFormatChannel_Y][i] / w - expected.y) < 1e-3f);
                    AM_EXPECT(std::abs((*field)[eBFormatChannel_Z][i] / w - expected.z) < 1e-3f);
                }
            }

            AM_EXPECT(found);
        }
    };

    AM_REGISTER_TEST(mixer_nodes, reflections_arrive_from_the_nearest_wall);
} // namespace SparkyStudios::Audio::Amplitude::Tests
