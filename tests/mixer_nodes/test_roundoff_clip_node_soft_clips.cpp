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
#include <Mixer/Nodes/RoundoffClipNode.h>

#include "NodeTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(NodeTestCase, mixer_nodes, roundoff_clip_node_soft_clips)
    {
    public:
        void Run() override
        {
            constexpr AmUInt64 frameCount = 5;
            constexpr AmUInt16 channelCount = 1;

            auto instance = CreateAndConfigureNode<RoundoffClipNode>(frameCount, channelCount);
            auto* processor = AsProcessor(instance);
            AM_EXPECT_NOT(processor == nullptr);

            AudioBuffer input(frameCount, channelCount);
            input[0][0] = 0.0f;
            input[0][1] = 0.5f;
            input[0][2] = 1.0f;
            input[0][3] = -0.5f;
            input[0][4] = -1.0f;

            const AudioBuffer* output = processor->Process(&input);
            AM_EXPECT_NOT(output == nullptr);

            auto cubic = [](AmReal32 x) -> AmReal32 { return 0.87f * x - 0.1f * x * x * x; };

            AM_EXPECT(std::abs((*output)[0][0] - cubic(0.0f)) < 1e-5f);
            AM_EXPECT(std::abs((*output)[0][1] - cubic(0.5f)) < 1e-5f);
            AM_EXPECT(std::abs((*output)[0][2] - cubic(1.0f)) < 1e-5f);
            AM_EXPECT(std::abs((*output)[0][3] - cubic(-0.5f)) < 1e-5f);
            AM_EXPECT(std::abs((*output)[0][4] - cubic(-1.0f)) < 1e-5f);

            AM_EXPECT(std::abs((*output)[0][2]) < std::abs(input[0][2]));
        }
    };

    AM_REGISTER_TEST(mixer_nodes, roundoff_clip_node_soft_clips);
} // namespace SparkyStudios::Audio::Amplitude::Tests
