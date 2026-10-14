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
        ChannelInstanceData MakeInstance(AmReal32 weight)
        {
            ChannelInstanceData data{};
            data.weight = weight;

            return data;
        }

        ChannelInstanceData WithObstruction(ChannelInstanceData data, AmReal32 obstruction)
        {
            data.hasObstruction = true;
            data.obstruction = obstruction;

            return data;
        }

        ChannelInstanceData WithOcclusion(ChannelInstanceData data, AmReal32 occlusion)
        {
            data.hasOcclusion = true;
            data.occlusion = occlusion;

            return data;
        }
    } // namespace

    // In separate mode, each instance is rendered on its own: it gets its own value, and the channel's only when it
    // has none. An explicit 0 is a value: it does not fall back to the channel's.
    AM_TEST_CASE(ComponentTestCase, mixer_instances, layer_applies_instance_values_in_separate_mode)
    {
    public:
        void Run() override
        {
            AmplimixLayerImpl layer;
            layer.obstruction.store(0.3f);
            layer.occlusion.store(0.6f);

            layer.instanceData = {
                MakeInstance(1.0f),
                WithObstruction(MakeInstance(1.0f), 0.8f),
                WithOcclusion(MakeInstance(1.0f), 0.4f),
                WithOcclusion(WithObstruction(MakeInstance(1.0f), 0.0f), 0.0f),
            };
            layer.processingInstance = true;

            // No value of its own: the channel's.
            layer.currentInstanceIndex = 0;
            AM_EXPECT_EQ(layer.GetObstruction(), 0.3f);
            AM_EXPECT_EQ(layer.GetOcclusion(), 0.6f);

            // Each quantity is set independently.
            layer.currentInstanceIndex = 1;
            AM_EXPECT_EQ(layer.GetObstruction(), 0.8f);
            AM_EXPECT_EQ(layer.GetOcclusion(), 0.6f);

            layer.currentInstanceIndex = 2;
            AM_EXPECT_EQ(layer.GetObstruction(), 0.3f);
            AM_EXPECT_EQ(layer.GetOcclusion(), 0.4f);

            // An explicit zero is instance-only: the channel's value does not apply.
            layer.currentInstanceIndex = 3;
            AM_EXPECT_EQ(layer.GetObstruction(), 0.0f);
            AM_EXPECT_EQ(layer.GetOcclusion(), 0.0f);
        }
    };

    // In blended mode, the instances are one sound: the layer renders with their weighted average, and keeps the
    // channel's own value while no instance has one.
    AM_TEST_CASE(ComponentTestCase, mixer_instances, layer_blends_instance_values_in_blended_mode)
    {
    public:
        void Run() override
        {
            AmplimixLayerImpl layer;
            layer.obstruction.store(0.2f);
            layer.occlusion.store(0.5f);

            // Nothing set on the instances: the channel's values apply as they always did.
            layer.instanceData = { MakeInstance(1.0f), MakeInstance(1.0f) };
            layer.UpdateBlendedInstanceValues();
            AM_EXPECT_EQ(layer.GetObstruction(), 0.2f);
            AM_EXPECT_EQ(layer.GetOcclusion(), 0.5f);

            // One instance is blocked, the other follows the channel's 0.2: with equal weights and no attenuation, the
            // average of 1.0 and 0.2. The occlusion, set on no instance, stays the channel's.
            layer.instanceData = { WithObstruction(MakeInstance(1.0f), 1.0f), MakeInstance(1.0f) };
            layer.UpdateBlendedInstanceValues();
            AM_EXPECT(std::abs(layer.GetObstruction() - 0.6f) < 1e-5f);
            AM_EXPECT_EQ(layer.GetOcclusion(), 0.5f);

            // The weights count: 3 : 1 gives (3 * 1.0 + 0.2) / 4.
            layer.instanceData = { WithObstruction(MakeInstance(3.0f), 1.0f), MakeInstance(1.0f) };
            layer.UpdateBlendedInstanceValues();
            AM_EXPECT(std::abs(layer.GetObstruction() - 0.8f) < 1e-5f);

            // Once the instances are gone, the channel's value applies again.
            layer.instanceData.clear();
            layer.UpdateBlendedInstanceValues();
            AM_EXPECT_EQ(layer.GetObstruction(), 0.2f);
        }
    };

    AM_REGISTER_TEST(mixer_instances, layer_applies_instance_values_in_separate_mode);
    AM_REGISTER_TEST(mixer_instances, layer_blends_instance_values_in_blended_mode);
} // namespace SparkyStudios::Audio::Amplitude::Tests
