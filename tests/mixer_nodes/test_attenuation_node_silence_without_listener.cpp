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

#include <Mixer/Nodes/AttenuationNode.h>

#include "NodeTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(NodeTestCase, mixer_nodes, attenuation_node_silence_without_listener)
    {
    public:
        void Run() override
        {
            constexpr AmUInt64 frameCount = 256;
            constexpr AmUInt16 channelCount = 1;

            // Set an attenuation but leave listener as default (invalid)
            MockAttenuation mockAttenuation(100.0);
            GetMockLayer().SetAttenuation(&mockAttenuation);
            GetMockLayer().SetSpatialization(eSpatialization_Position);

            auto instance = CreateAndConfigureNode<AttenuationNode>(frameCount, channelCount);
            auto* processor = AsProcessor(instance);
            AM_EXPECT_NOT(processor == nullptr);

            AudioBuffer input(frameCount, channelCount);
            for (AmUInt64 i = 0; i < frameCount; ++i)
                input[0][i] = std::sin(2.0f * AM_PI32 * static_cast<AmReal32>(i) / static_cast<AmReal32>(frameCount));

            const AudioBuffer* output = processor->Process(&input);

            // Without a valid listener, attenuation should produce silence (nullptr)
            AM_EXPECT(output == nullptr);
        }
    };

    AM_REGISTER_TEST(mixer_nodes, attenuation_node_silence_without_listener);
} // namespace SparkyStudios::Audio::Amplitude::Tests
