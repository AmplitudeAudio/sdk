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

#include <Mixer/Nodes/StereoPanningNode.h>

#include "NodeTestCase.h"
#include "TestRegistry.h"
#include "TestUtils.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(NodeTestCase, mixer_nodes, stereo_panning_node_pans_to_left)
    {
    public:
        void Run() override
        {
            constexpr AmUInt64 frameCount = 256;
            constexpr AmUInt16 monoChannels = 1;

            // Listener at origin, looking along -Z, up is +Y
            ListenerInternalState listenerState;
            listenerState.SetLocation({ 0.0f, 0.0f, 0.0f });
            listenerState.SetOrientation(Orientation({ 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f }));
            listenerState.Update();
            Listener listener = InitTestListener(&listenerState);
            GetMockLayer().SetListener(listener);

            // Source to the left of the listener
            GetMockLayer().SetSpatialization(eSpatialization_Position);
            GetMockLayer().SetLocation({ -10.0f, 0.0f, 0.0f });

            auto instance = CreateAndConfigureNode<StereoPanningNode>(frameCount, monoChannels);
            auto* processor = AsProcessor(instance);
            AM_EXPECT_NOT(processor == nullptr);

            AudioBuffer input(frameCount, monoChannels);
            for (AmUInt64 i = 0; i < frameCount; ++i)
                input[0][i] = 1.0f;

            const AudioBuffer* output = processor->Process(&input);
            AM_EXPECT_NOT(output == nullptr);
            AM_EXPECT_EQ(output->GetChannelCount(), 2);

            // Compute average energy per channel
            AmReal32 leftEnergy = 0.0f;
            AmReal32 rightEnergy = 0.0f;
            for (AmUInt64 i = 0; i < frameCount; ++i)
            {
                leftEnergy += (*output)[0][i] * (*output)[0][i];
                rightEnergy += (*output)[1][i] * (*output)[1][i];
            }

            // Source to the left -> left channel should have more energy
            AM_EXPECT(leftEnergy > rightEnergy);
        }
    };

    AM_REGISTER_TEST(mixer_nodes, stereo_panning_node_pans_to_left);
} // namespace SparkyStudios::Audio::Amplitude::Tests
