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
    AM_TEST_CASE(NodeTestCase, mixer_nodes, reverb_node_dispatch)
    {
    public:
        void Run() override
        {
            // 1. Dispatch to Dattorro
            {
                auto node = amshared(ReverbNode, "Dattorro");
                AM_EXPECT_NOT(node == nullptr);
                AM_EXPECT_EQ(node->GetName(), "Dattorro");
                AM_EXPECT_EQ(node->GetAlgorithmName(), "Dattorro");

                auto instance = node->CreateInstance();
                AM_EXPECT_NOT(instance == nullptr);
                instance->Initialize(1, GetLayer(), nullptr, 0);
                instance->Configure(512, 2);
                instance->Reset();
            }

            // 2. Dispatch to Freeverb
            {
                auto node = amshared(ReverbNode, "Freeverb");
                AM_EXPECT_NOT(node == nullptr);
                AM_EXPECT_EQ(node->GetName(), "Freeverb");
                AM_EXPECT_EQ(node->GetAlgorithmName(), "Freeverb");

                auto instance = node->CreateInstance();
                AM_EXPECT_NOT(instance == nullptr);
                instance->Initialize(2, GetLayer(), nullptr, 0);
                instance->Configure(512, 2);
                instance->Reset();
            }

            // 3. Dispatch to RoomReverb
            {
                auto node = amshared(ReverbNode, "RoomReverb");
                AM_EXPECT_NOT(node == nullptr);
                AM_EXPECT_EQ(node->GetName(), "RoomReverb");
                AM_EXPECT_EQ(node->GetAlgorithmName(), "RoomReverb");

                auto instance = node->CreateInstance();
                AM_EXPECT_NOT(instance == nullptr);
                instance->Initialize(3, GetLayer(), nullptr, 0);
                instance->Configure(512, 2);
                instance->Reset();
            }

            // 4. Convenience nodes registered in Engine
            {
                auto reverbNode = std::dynamic_pointer_cast<ReverbNode>(Node::Find("Reverb"));
                AM_EXPECT_NOT(reverbNode == nullptr);
                AM_EXPECT_EQ(reverbNode->GetAlgorithmName(), "Freeverb");

                auto freeverbNode = std::dynamic_pointer_cast<ReverbNode>(Node::Find("Freeverb"));
                AM_EXPECT_NOT(freeverbNode == nullptr);
                AM_EXPECT_EQ(freeverbNode->GetAlgorithmName(), "Freeverb");

                auto dattorroNode = std::dynamic_pointer_cast<ReverbNode>(Node::Find("Dattorro"));
                AM_EXPECT_NOT(dattorroNode == nullptr);
                AM_EXPECT_EQ(dattorroNode->GetAlgorithmName(), "Dattorro");

                auto roomReverbNode = std::dynamic_pointer_cast<ReverbNode>(Node::Find("RoomReverb"));
                AM_EXPECT_NOT(roomReverbNode == nullptr);
                AM_EXPECT_EQ(roomReverbNode->GetAlgorithmName(), "RoomReverb");
            }
        }
    };

    AM_REGISTER_TEST(mixer_nodes, reverb_node_dispatch);
} // namespace SparkyStudios::Audio::Amplitude::Tests
