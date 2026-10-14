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

#include <Mixer/Amplimix.h>

#include "ComponentTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    namespace
    {
        ChannelInstanceData MakeInstance()
        {
            ChannelInstanceData data{};
            data.weight = 1.0f;

            return data;
        }
    } // namespace

    // The layer must answer from the state it took from the snapshot at the start of the block. It has no channel here
    // at all: anything it still read from the game-owned channel state would be missing.
    AM_TEST_CASE(ComponentTestCase, mixer_instances, layer_follows_instancing_state_from_snapshot)
    {
    public:
        void Run() override
        {
            AmplimixLayerImpl layer;

            // Nothing taken from a snapshot yet.
            AM_EXPECT_NOT(layer.IsMultiPosition());
            AM_EXPECT(layer.GetInstancingMode() == eChannelInstanceMode_Blended);
            AM_EXPECT_EQ(layer.GetInstanceCount(), 0u);

            // Instancing is on, and two instances came with the snapshot.
            layer.instancingEnabled = true;
            layer.instanceData = { MakeInstance(), MakeInstance() };
            AM_EXPECT(layer.IsMultiPosition());
            AM_EXPECT_EQ(layer.GetInstanceCount(), 2u);

            // While one instance is processed on its own, the layer is not a multi-position source.
            layer.processingInstance = true;
            AM_EXPECT_NOT(layer.IsMultiPosition());
            layer.processingInstance = false;

            // The mode comes from the snapshot too.
            layer.instancingMode = eChannelInstanceMode_Separate;
            AM_EXPECT(layer.GetInstancingMode() == eChannelInstanceMode_Separate);

            // Instancing on without instances is not multi-position.
            layer.instanceData.clear();
            AM_EXPECT_NOT(layer.IsMultiPosition());

            // And instances without instancing (a stale buffer) are not either.
            layer.instanceData = { MakeInstance() };
            layer.instancingEnabled = false;
            AM_EXPECT_NOT(layer.IsMultiPosition());
        }
    };

    AM_REGISTER_TEST(mixer_instances, layer_follows_instancing_state_from_snapshot);
} // namespace SparkyStudios::Audio::Amplitude::Tests
