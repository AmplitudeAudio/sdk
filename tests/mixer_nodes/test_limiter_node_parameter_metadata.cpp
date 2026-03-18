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
#include <Mixer/Nodes/LimiterNode.h>

#include "NodeTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(NodeTestCase, mixer_nodes, limiter_node_parameter_metadata)
    {
    public:
        void Run() override
        {
            LimiterNode node;

            AM_EXPECT_EQ(AmString("Limiter"), node.GetName());
            AM_EXPECT_EQ(static_cast<AmSize>(3), node.GetParameterCount());
            AM_EXPECT(node.CanConsume());
            AM_EXPECT(node.CanProduce());
            AM_EXPECT_EQ(static_cast<AmSize>(1), node.GetMaxInputCount());
            AM_EXPECT_EQ(static_cast<AmSize>(1), node.GetMinInputCount());

            AM_EXPECT_EQ(AmString("Threshold (dB)"), node.GetParameterName(0));
            AM_EXPECT(std::abs(node.GetParameterMin(0) - (-60.0f)) < 1e-6f);
            AM_EXPECT(std::abs(node.GetParameterMax(0) - 0.0f) < 1e-6f);

            AM_EXPECT_EQ(AmString("Attack (ms)"), node.GetParameterName(1));
            AM_EXPECT(std::abs(node.GetParameterMin(1) - 0.1f) < 1e-6f);
            AM_EXPECT(std::abs(node.GetParameterMax(1) - 100.0f) < 1e-6f);

            AM_EXPECT_EQ(AmString("Release (ms)"), node.GetParameterName(2));
            AM_EXPECT(std::abs(node.GetParameterMin(2) - 20.0f) < 1e-6f);
            AM_EXPECT(std::abs(node.GetParameterMax(2) - 2000.0f) < 1e-6f);

            AM_EXPECT_EQ(eParameterType_Float, node.GetParameterType(0));
            AM_EXPECT_EQ(eParameterType_Float, node.GetParameterType(1));
            AM_EXPECT_EQ(eParameterType_Float, node.GetParameterType(2));
        }
    };

    AM_REGISTER_TEST(mixer_nodes, limiter_node_parameter_metadata);
} // namespace SparkyStudios::Audio::Amplitude::Tests
