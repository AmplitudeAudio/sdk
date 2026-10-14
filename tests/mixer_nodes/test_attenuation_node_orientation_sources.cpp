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
#include "TestUtils.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    namespace
    {
        bool SameForward(const Orientation& a, const Orientation& b)
        {
            return std::abs(a.GetForward().x - b.GetForward().x) < 1e-4f && std::abs(a.GetForward().y - b.GetForward().y) < 1e-4f &&
                std::abs(a.GetForward().z - b.GetForward().z) < 1e-4f;
        }
    } // namespace

    class AttenuationOrientationTestCase : public NodeTestCase
    {
    protected:
        // Runs one block of the attenuation node for a listener at the origin.
        void ProcessOnce(MockAttenuation& attenuation)
        {
            constexpr AmUInt64 frameCount = 256;
            constexpr AmUInt16 channelCount = 1;

            GetMockLayer().SetSampleRate(48000);

            ListenerInternalState listenerState;
            listenerState.SetLocation({ 0.0f, 0.0f, 0.0f });
            listenerState.SetOrientation(Orientation({ 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f }));
            listenerState.Update();
            GetMockLayer().SetListener(InitTestListener(&listenerState));
            GetMockLayer().SetAttenuation(&attenuation);

            AudioBuffer input(frameCount, channelCount);
            for (AmUInt64 i = 0; i < frameCount; ++i)
                input[0][i] = 1.0f;

            auto instance = CreateAndConfigureNode<AttenuationNode>(frameCount, channelCount);
            auto* processor = AsProcessor(instance);
            AM_EXPECT_NOT(processor == nullptr);

            if (processor != nullptr)
                AM_UNUSED(processor->Process(&input));
        }
    };

    // Blended instances sit at arbitrary world positions: the entity's orientation means nothing for them, so
    // only an instance with an orientation of its own is evaluated with one.
    AM_TEST_CASE(AttenuationOrientationTestCase, mixer_nodes, attenuation_node_blended_applies_only_explicit_instance_orientation)
    {
    public:
        void Run() override
        {
            EntityInternalState entityState;
            entityState.SetOrientation(Orientation(1.0f, 0.0f, 0.0f));
            GetMockLayer().SetEntity(InitTestEntity(&entityState));

            const Orientation instanceOrientation(-0.6f, 0.0f, 0.0f);

            GetMockLayer().SetSpatialization(eSpatialization_Position);
            GetMockLayer().SetMultiPosition(true);
            GetMockLayer().AddInstance({ 0.0f, 0.0f, -10.0f }, 0.5f, 1.0f);
            GetMockLayer().AddInstance({ 0.0f, 0.0f, -20.0f }, 0.5f, 1.0f);
            GetMockLayer().SetInstanceOrientation(1, instanceOrientation);

            MockAttenuation attenuation(100.0);
            ProcessOnce(attenuation);

            AM_EXPECT_EQ(attenuation.GetOrientedGainCount(), 1u);
            AM_EXPECT(attenuation.GetLastOrientation().has_value());
            AM_EXPECT(SameForward(*attenuation.GetLastOrientation(), instanceOrientation));
        }
    };

    // A separate-mode instance with its own orientation is attenuated with it, not with the entity's.
    AM_TEST_CASE(AttenuationOrientationTestCase, mixer_nodes, attenuation_node_separate_mode_uses_instance_orientation)
    {
    public:
        void Run() override
        {
            const Orientation entityOrientation(1.0f, 0.0f, 0.0f);
            const Orientation instanceOrientation(-0.6f, 0.0f, 0.0f);

            EntityInternalState entityState;
            entityState.SetLocation({ 0.0f, 0.0f, -10.0f });
            entityState.SetOrientation(entityOrientation);
            GetMockLayer().SetEntity(InitTestEntity(&entityState));

            GetMockLayer().SetSpatialization(eSpatialization_PositionOrientation);
            GetMockLayer().SetLocation({ 0.0f, 0.0f, -30.0f });
            GetMockLayer().SetOrientation(instanceOrientation);

            MockAttenuation attenuation(100.0);
            ProcessOnce(attenuation);

            AM_EXPECT_EQ(attenuation.GetOrientedGainCount(), 1u);
            AM_EXPECT(attenuation.GetLastOrientation().has_value());
            AM_EXPECT(SameForward(*attenuation.GetLastOrientation(), instanceOrientation));
            AM_EXPECT_NOT(SameForward(*attenuation.GetLastOrientation(), entityOrientation));
        }
    };

    // Without an orientation of its own, an instance keeps following the entity's, as it did before instances had one.
    AM_TEST_CASE(AttenuationOrientationTestCase, mixer_nodes, attenuation_node_separate_mode_follows_entity_without_instance_orientation)
    {
    public:
        void Run() override
        {
            const Orientation entityOrientation(1.0f, 0.0f, 0.0f);

            EntityInternalState entityState;
            entityState.SetLocation({ 0.0f, 0.0f, -10.0f });
            entityState.SetOrientation(entityOrientation);
            GetMockLayer().SetEntity(InitTestEntity(&entityState));

            GetMockLayer().SetSpatialization(eSpatialization_PositionOrientation);
            GetMockLayer().SetLocation({ 0.0f, 0.0f, -30.0f });

            MockAttenuation attenuation(100.0);
            ProcessOnce(attenuation);

            AM_EXPECT_EQ(attenuation.GetOrientedGainCount(), 1u);
            AM_EXPECT(attenuation.GetLastOrientation().has_value());
            AM_EXPECT(SameForward(*attenuation.GetLastOrientation(), entityOrientation));
        }
    };

    AM_REGISTER_TEST(mixer_nodes, attenuation_node_blended_applies_only_explicit_instance_orientation);
    AM_REGISTER_TEST(mixer_nodes, attenuation_node_separate_mode_uses_instance_orientation);
    AM_REGISTER_TEST(mixer_nodes, attenuation_node_separate_mode_follows_entity_without_instance_orientation);
} // namespace SparkyStudios::Audio::Amplitude::Tests
