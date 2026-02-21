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
#include <Mixer/Nodes/ClampNode.h>

#include "NodeTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(NodeTestCase, mixer_nodes, clamp_node_preserves_silence)
    {
    public:
        void Run() override
        {
            constexpr AmUInt64 frameCount = 128;
            constexpr AmUInt16 channelCount = 2;

            auto instance = CreateAndConfigureNode<ClampNode>(frameCount, channelCount);
            auto* processor = AsProcessor(instance);
            AM_EXPECT_NOT(processor == nullptr);

            // All-zero input
            AudioBuffer input(frameCount, channelCount);
            input.Clear();

            const AudioBuffer* output = processor->Process(&input);
            AM_EXPECT_NOT(output == nullptr);

            // All outputs should be exactly zero
            for (AmUInt16 c = 0; c < channelCount; ++c)
                for (AmUInt64 i = 0; i < frameCount; ++i)
                    AM_EXPECT(std::abs((*output)[c][i]) < 1e-6f);
        }
    };

    AM_REGISTER_TEST(mixer_nodes, clamp_node_preserves_silence);
} // namespace SparkyStudios::Audio::Amplitude::Tests
