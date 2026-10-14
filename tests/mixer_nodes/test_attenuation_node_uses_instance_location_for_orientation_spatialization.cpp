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
    AM_TEST_CASE(NodeTestCase, mixer_nodes, attenuation_node_uses_instance_location_for_orientation_spatialization)
    {
    public:
        void Run() override
        {
            constexpr AmUInt64 frameCount = 1024;
            constexpr AmUInt16 channelCount = 1;
            constexpr AmUInt32 sampleRate = 48000;
            constexpr AmReal64 maxDistance = 100.0;

            GetMockLayer().SetSampleRate(sampleRate);
            GetMockLayer().SetSpatialization(eSpatialization_PositionOrientation);

            ListenerInternalState listenerState;
            listenerState.SetLocation({ 0.0f, 0.0f, 0.0f });
            listenerState.SetOrientation(Orientation({ 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f }));
            listenerState.Update();
            GetMockLayer().SetListener(InitTestListener(&listenerState));

            // The entity is close to the listener, while the layer is processing a separate-mode instance far from it.
            // In separate mode the layer is not multi-position, and its location is the instance's.
            EntityInternalState entityState;
            entityState.SetLocation({ 0.0f, 0.0f, -10.0f });
            GetMockLayer().SetEntity(InitTestEntity(&entityState));
            GetMockLayer().SetLocation({ 0.0f, 0.0f, -80.0f });
            GetMockLayer().SetMultiPosition(false);

            MockAttenuation mockAttenuation(maxDistance);
            GetMockLayer().SetAttenuation(&mockAttenuation);

            AudioBuffer input(frameCount, channelCount);
            for (AmUInt64 i = 0; i < frameCount; ++i)
                input[0][i] = std::sin(2.0f * AM_PI32 * 440.0f * static_cast<AmReal32>(i) / static_cast<AmReal32>(sampleRate));

            auto instance = CreateAndConfigureNode<AttenuationNode>(frameCount, channelCount);
            auto* processor = AsProcessor(instance);
            AM_EXPECT_NOT(processor == nullptr);

            const AudioBuffer* output = processor->Process(&input);
            AM_EXPECT_NOT(output == nullptr);

            AmReal32 outputPeak = 0.0f;
            AmReal32 inputPeak = 0.0f;
            for (AmUInt64 i = 0; i < frameCount; ++i)
            {
                outputPeak = std::max(outputPeak, std::abs((*output)[0][i]));
                inputPeak = std::max(inputPeak, std::abs(input[0][i]));
            }

            // Gain at the instance location: 1 - 80 / 100 = 0.2. The entity location would give 0.9.
            const AmReal32 actualGain = outputPeak / inputPeak;
            AM_EXPECT(std::abs(actualGain - 0.2f) < 0.02f);
        }
    };

    AM_REGISTER_TEST(mixer_nodes, attenuation_node_uses_instance_location_for_orientation_spatialization);
} // namespace SparkyStudios::Audio::Amplitude::Tests
