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
    AM_TEST_CASE(NodeTestCase, mixer_nodes, attenuation_node_gain_decreases_with_distance)
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

            // Generate consistent input signal
            AudioBuffer input(frameCount, channelCount);
            for (AmUInt64 i = 0; i < frameCount; ++i)
                input[0][i] = std::sin(2.0f * AM_PI32 * 440.0f * static_cast<AmReal32>(i) / static_cast<AmReal32>(sampleRate));

            AmReal32 inputPeak = 0.0f;
            for (AmUInt64 i = 0; i < frameCount; ++i)
                inputPeak = std::max(inputPeak, std::abs(input[0][i]));

            // Test at increasing distances: gain should monotonically decrease
            struct TestCase
            {
                AmReal32 distance;
                const char* label;
            };

            std::vector<TestCase> testCases = {
                { 5.0f,  "near" },
                { 30.0f, "medium" },
                { 70.0f, "far" },
            };

            AmReal32 previousPeak = inputPeak; // First distance should attenuate below input

            for (const auto& tc : testCases)
            {
                // Place sound source at given distance along Z axis
                GetMockLayer().SetLocation({ 0.0f, 0.0f, -tc.distance });

                // Need fresh node instance for each distance (avoids internal state carryover)
                auto instance = CreateAndConfigureNode<AttenuationNode>(frameCount, channelCount);
                auto* processor = AsProcessor(instance);
                AM_EXPECT_NOT(processor == nullptr);

                const AudioBuffer* output = processor->Process(&input);
                AM_EXPECT_NOT(output == nullptr);

                // Measure output peak
                AmReal32 outputPeak = 0.0f;
                for (AmUInt64 i = 0; i < frameCount; ++i)
                    outputPeak = std::max(outputPeak, std::abs((*output)[0][i]));

                // Output should be attenuated relative to input
                AM_EXPECT(outputPeak < inputPeak);

                // Gain should decrease monotonically with distance
                AM_EXPECT(outputPeak < previousPeak);

                previousPeak = outputPeak;
            }
        }
    };

    AM_REGISTER_TEST(mixer_nodes, attenuation_node_gain_decreases_with_distance);
} // namespace SparkyStudios::Audio::Amplitude::Tests
