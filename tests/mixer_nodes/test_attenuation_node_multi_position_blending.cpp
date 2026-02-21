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
    AM_TEST_CASE(NodeTestCase, mixer_nodes, attenuation_node_multi_position_blending)
    {
    public:
        void Run() override
        {
            constexpr AmUInt64 frameCount = 1024;
            constexpr AmUInt16 channelCount = 1;
            constexpr AmUInt32 sampleRate = 48000;
            constexpr AmReal64 maxDistance = 100.0;

            GetMockLayer().SetSampleRate(sampleRate);
            GetMockLayer().SetSpatialization(eSpatialization_Position);

            // Create a valid listener at origin
            ListenerInternalState listenerState;
            listenerState.SetLocation({ 0.0f, 0.0f, 0.0f });
            listenerState.SetOrientation(Orientation({ 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f }));
            listenerState.Update();
            Listener listener = InitTestListener(&listenerState);
            GetMockLayer().SetListener(listener);

            MockAttenuation mockAttenuation(maxDistance);
            GetMockLayer().SetAttenuation(&mockAttenuation);

            // Enable multi-position mode with two instances:
            // - One near (distance 10, gain ~0.9), weight 0.5
            // - One far (distance 80, gain ~0.2), weight 0.5
            GetMockLayer().SetMultiPosition(true);
            GetMockLayer().AddInstance({ 0.0f, 0.0f, -10.0f }, 0.5f, 1.0f);
            GetMockLayer().AddInstance({ 0.0f, 0.0f, -80.0f }, 0.5f, 1.0f);

            // Generate input signal
            AudioBuffer input(frameCount, channelCount);
            for (AmUInt64 i = 0; i < frameCount; ++i)
                input[0][i] = std::sin(2.0f * AM_PI32 * 440.0f * static_cast<AmReal32>(i) / static_cast<AmReal32>(sampleRate));

            auto instance = CreateAndConfigureNode<AttenuationNode>(frameCount, channelCount);
            auto* processor = AsProcessor(instance);
            AM_EXPECT_NOT(processor == nullptr);

            const AudioBuffer* output = processor->Process(&input);
            AM_EXPECT_NOT(output == nullptr);

            // Calculate output peak
            AmReal32 outputPeak = 0.0f;
            for (AmUInt64 i = 0; i < frameCount; ++i)
                outputPeak = std::max(outputPeak, std::abs((*output)[0][i]));

            AmReal32 inputPeak = 0.0f;
            for (AmUInt64 i = 0; i < frameCount; ++i)
                inputPeak = std::max(inputPeak, std::abs(input[0][i]));

            // The blended gain should be the average of near and far gains:
            // near gain = 1 - 10/100 = 0.9, far gain = 1 - 80/100 = 0.2
            // blended = (0.9 * 0.5 + 0.2 * 0.5) / (0.5 + 0.5) = 0.55
            AmReal32 expectedGain = 0.55f;
            AmReal32 actualGain = outputPeak / inputPeak;

            // Linear attenuation with no filters: tight tolerance is appropriate
            AM_EXPECT(std::abs(actualGain - expectedGain) < 0.02f);

            // Output should be less than input (attenuated)
            AM_EXPECT(outputPeak < inputPeak);
        }
    };

    AM_REGISTER_TEST(mixer_nodes, attenuation_node_multi_position_blending);
} // namespace SparkyStudios::Audio::Amplitude::Tests
