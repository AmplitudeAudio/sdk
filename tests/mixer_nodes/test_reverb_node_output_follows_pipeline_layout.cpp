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

#include "NodeTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(NodeTestCase, mixer_nodes, reverb_node_output_follows_pipeline_layout)
    {
    public:
        void Run() override
        {
            for (const eChannelLayout layout : { eChannelLayout_Mono, eChannelLayout_Stereo, eChannelLayout_Surround_7_1 })
            {
                ReverbNode node;
                auto instance = node.CreateInstance();
                instance->Initialize(1, GetLayer(), nullptr, node.GetParameterCount());
                instance->SetPipelineOutputLayout(layout);
                instance->Configure(512, 1);

                AM_EXPECT_EQ(ChannelLayout::Get(layout).GetChannelCount(), instance->GetOutputChannelCount());

                // No room: nothing to render, and nothing breaks.
                AudioBuffer input(512, 1);
                AM_EXPECT(AsProcessor(instance)->Process(&input) == nullptr);
            }
        }
    };

    AM_REGISTER_TEST(mixer_nodes, reverb_node_output_follows_pipeline_layout);
} // namespace SparkyStudios::Audio::Amplitude::Tests
