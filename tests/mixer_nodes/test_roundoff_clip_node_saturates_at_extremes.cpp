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
    AM_TEST_CASE(NodeTestCase, mixer_nodes, roundoff_clip_node_saturates_at_extremes)
    {
    public:
        void Run() override
        {
            constexpr AmUInt64 frameCount = 4;
            constexpr AmUInt16 channelCount = 1;
            constexpr AmReal32 kSaturationLevel = 0.9862875f;

            auto instance = CreateAndConfigureNode<RoundoffClipNode>(frameCount, channelCount);
            auto* processor = AsProcessor(instance);
            AM_EXPECT_NOT(processor == nullptr);

            AudioBuffer input(frameCount, channelCount);
            input[0][0] = 2.0f;
            input[0][1] = -2.0f;
            input[0][2] = 10.0f;
            input[0][3] = -100.0f;

            const AudioBuffer* output = processor->Process(&input);
            AM_EXPECT_NOT(output == nullptr);

            AM_EXPECT(std::abs((*output)[0][0] - kSaturationLevel) < 1e-5f);
            AM_EXPECT(std::abs((*output)[0][1] - (-kSaturationLevel)) < 1e-5f);
            AM_EXPECT(std::abs((*output)[0][2] - kSaturationLevel) < 1e-5f);
            AM_EXPECT(std::abs((*output)[0][3] - (-kSaturationLevel)) < 1e-5f);
        }
    };

    AM_REGISTER_TEST(mixer_nodes, roundoff_clip_node_saturates_at_extremes);
} // namespace SparkyStudios::Audio::Amplitude::Tests
