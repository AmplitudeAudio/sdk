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

#include <optional>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Mixer/InstanceBlending.h>

#include "ComponentTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    namespace
    {
        // An instance at the given distance from the listener (on the X axis), with an optional obstruction.
        ChannelInstanceData MakeInstance(AmReal32 distance, AmReal32 weight, const std::optional<AmReal32>& obstruction = std::nullopt)
        {
            ChannelInstanceData data{};
            data.location = { distance, 0.0f, 0.0f };
            data.weight = weight;
            data.hasObstruction = obstruction.has_value();
            data.obstruction = obstruction.value_or(0.0f);

            return data;
        }

        // A gain that falls with the distance to a listener at the origin.
        AmReal32 DistanceGain(const ChannelInstanceData& instance)
        {
            return 1.0f / (1.0f + instance.location.x);
        }

        bool Blend(const std::vector<ChannelInstanceData>& instances, AmReal32 fallback, AmReal32& result)
        {
            return BlendInstanceValues(
                instances, fallback, &ChannelInstanceData::obstruction, &ChannelInstanceData::hasObstruction, DistanceGain, result);
        }
    } // namespace

    // In blended mode the instances are one sound, so their obstructions are averaged. A near instance is audible and
    // dominates, a far one barely counts.
    AM_TEST_CASE(ComponentTestCase, mixer_instances, blend_instance_values_weights_by_audible_contribution)
    {
    public:
        void Run() override
        {
            // Near and clear (gain 0.5), far and blocked (gain 0.1): (0 * 0.5 + 1 * 0.1) / 0.6.
            {
                AmReal32 result = -1.0f;
                AM_EXPECT(Blend({ MakeInstance(1.0f, 1.0f, 0.0f), MakeInstance(9.0f, 1.0f, 1.0f) }, 0.0f, result));
                AM_EXPECT(std::abs(result - 0.1f / 0.6f) < 1e-5f);
            }

            // The other way around: the near blocked instance dominates.
            {
                AmReal32 result = -1.0f;
                AM_EXPECT(Blend({ MakeInstance(1.0f, 1.0f, 1.0f), MakeInstance(9.0f, 1.0f, 0.0f) }, 0.0f, result));
                AM_EXPECT(std::abs(result - 0.5f / 0.6f) < 1e-5f);
            }

            // The weight of an instance scales its contribution: at the same distance, 3 : 1.
            {
                AmReal32 result = -1.0f;
                AM_EXPECT(Blend({ MakeInstance(1.0f, 3.0f, 1.0f), MakeInstance(1.0f, 1.0f, 0.0f) }, 0.0f, result));
                AM_EXPECT(std::abs(result - 0.75f) < 1e-5f);
            }
        }
    };

    AM_TEST_CASE(ComponentTestCase, mixer_instances, blend_instance_values_counts_unset_instances_with_the_fallback)
    {
    public:
        void Run() override
        {
            // The instance without a value of its own counts with the entity's 0.2, at the same distance and weight.
            AmReal32 result = -1.0f;
            AM_EXPECT(Blend({ MakeInstance(1.0f, 1.0f, 1.0f), MakeInstance(1.0f, 1.0f) }, 0.2f, result));
            AM_EXPECT(std::abs(result - 0.6f) < 1e-5f);
        }
    };

    AM_TEST_CASE(ComponentTestCase, mixer_instances, blend_instance_values_leaves_the_channel_value_without_explicit_instances)
    {
    public:
        void Run() override
        {
            // Nothing set on any instance: the channel keeps its own value, and the result is untouched.
            AmReal32 result = -1.0f;
            AM_EXPECT_NOT(Blend({ MakeInstance(1.0f, 1.0f), MakeInstance(2.0f, 1.0f) }, 0.4f, result));
            AM_EXPECT_EQ(result, -1.0f);

            AM_EXPECT_NOT(Blend({}, 0.4f, result));
            AM_EXPECT_EQ(result, -1.0f);
        }
    };

    AM_TEST_CASE(ComponentTestCase, mixer_instances, blend_instance_values_falls_back_to_weights_when_nothing_is_audible)
    {
    public:
        void Run() override
        {
            const auto silent = [](const ChannelInstanceData&) -> AmReal32
            {
                return 0.0f;
            };

            // Every instance is inaudible: the weights alone decide, instead of dividing by nothing.
            AmReal32 result = -1.0f;
            const std::vector<ChannelInstanceData> instances = { MakeInstance(1.0f, 1.0f, 1.0f), MakeInstance(2.0f, 1.0f, 0.0f) };
            AM_EXPECT(BlendInstanceValues(
                instances, 0.0f, &ChannelInstanceData::obstruction, &ChannelInstanceData::hasObstruction, silent, result));
            AM_EXPECT(std::abs(result - 0.5f) < 1e-5f);
            AM_EXPECT(std::isfinite(result));
        }
    };

    AM_REGISTER_TEST(mixer_instances, blend_instance_values_weights_by_audible_contribution);
    AM_REGISTER_TEST(mixer_instances, blend_instance_values_counts_unset_instances_with_the_fallback);
    AM_REGISTER_TEST(mixer_instances, blend_instance_values_leaves_the_channel_value_without_explicit_instances);
    AM_REGISTER_TEST(mixer_instances, blend_instance_values_falls_back_to_weights_when_nothing_is_audible);
} // namespace SparkyStudios::Audio::Amplitude::Tests
