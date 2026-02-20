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
    AM_TEST_CASE(NodeTestCase, mixer_nodes, stereo_panning_node_center_pan)
    {
    public:
        void Run() override
        {
            constexpr AmUInt64 frameCount = 256;
            constexpr AmUInt16 monoChannels = 1;

            // Create a valid listener at origin, looking forward
            ListenerInternalState listenerState;
            listenerState.SetLocation({ 0.0f, 0.0f, 0.0f });
            listenerState.SetOrientation(Orientation({ 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f }));
            listenerState.Update();
            Listener listener = InitTestListener(&listenerState);
            GetMockLayer().SetListener(listener);

            // eSpatialization_None → center panning
            GetMockLayer().SetSpatialization(eSpatialization_None);

            auto instance = CreateAndConfigureNode<StereoPanningNode>(frameCount, monoChannels);
            auto* processor = AsProcessor(instance);
            AM_EXPECT_NOT(processor == nullptr);

            // Fill mono input with constant value
            AudioBuffer input(frameCount, monoChannels);
            for (AmUInt64 i = 0; i < frameCount; ++i)
                input[0][i] = 1.0f;

            const AudioBuffer* output = processor->Process(&input);
            AM_EXPECT_NOT(output == nullptr);
            AM_EXPECT_EQ(output->GetChannelCount(), 2);

            // Center pan: left = cos²(π/4) = 0.5, right = sin²(π/4) = 0.5
            for (AmUInt64 i = 0; i < frameCount; ++i)
            {
                AM_EXPECT(std::abs((*output)[0][i] - 0.5f) < 1e-4f);
                AM_EXPECT(std::abs((*output)[1][i] - 0.5f) < 1e-4f);
            }
        }
    };

    AM_REGISTER_TEST(mixer_nodes, stereo_panning_node_center_pan);
} // namespace SparkyStudios::Audio::Amplitude::Tests
