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

#include <Core/ListenerInternalState.h>
#include <Mixer/Nodes/OcclusionNode.h>

#include "LightEngineTestCase.h"
#include "NodeTestCase.h"
#include "TestRegistry.h"
#include "TestUtils.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(LightEngineTestCase, mixer_nodes, occlusion_node_reduces_gain_with_occlusion)
    {
    public:
        void SetUp() override
        {
            LightEngineTestCase::SetUp();
            InitializeConfig(AM_OS_STRING("tests.config.amconfig"));
        }

        void Run() override
        {
            constexpr AmUInt64 frameCount = 1024;
            constexpr AmUInt16 channelCount = 1;
            constexpr AmUInt32 sampleRate = 48000;

            // Create a valid listener
            ListenerInternalState listenerState;
            listenerState.SetLocation({ 0.0f, 0.0f, 0.0f });
            listenerState.SetOrientation(Orientation({ 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f }));
            listenerState.Update();
            Listener listener = InitTestListener(&listenerState);

            // Generate input signal
            AudioBuffer input(frameCount, channelCount);
            for (AmUInt64 i = 0; i < frameCount; ++i)
                input[0][i] = std::sin(2.0f * AM_PI32 * 440.0f * static_cast<AmReal32>(i) / static_cast<AmReal32>(sampleRate));

            AmReal32 inputRMS = 0.0f;
            for (AmUInt64 i = 0; i < frameCount; ++i)
                inputRMS += input[0][i] * input[0][i];
            inputRMS = std::sqrt(inputRMS / static_cast<AmReal32>(frameCount));

            // Test at different occlusion levels: gain should decrease
            // Occlusion gain curve: linear from (0,1) to (1,0)
            struct TestCase
            {
                AmReal32 occlusion;
                const char* label;
            };

            std::vector<TestCase> testCases = {
                { 0.0f, "no occlusion" },
                { 0.5f, "half occlusion" },
                { 1.0f, "full occlusion" },
            };

            AmReal32 previousRMS = inputRMS + 1.0f;

            for (const auto& tc : testCases)
            {
                SpatialMockLayer mockLayer;
                mockLayer.SetSampleRate(sampleRate);

                SoundFormat format;
                format.SetAll(sampleRate, channelCount, 32, frameCount, channelCount * 4, eAudioSampleFormat_Float32);
                mockLayer.SetSoundFormat(format);
                mockLayer.SetOcclusion(tc.occlusion);
                mockLayer.SetListener(listener);
                mockLayer.SetLocation({ 0.0f, 0.0f, -5.0f });
                mockLayer.SetSpatialization(eSpatialization_Position);

                OcclusionNode nodeFactory;
                auto instance = nodeFactory.CreateInstance();
                instance->Initialize(1, &mockLayer, nullptr, nodeFactory.GetParameterCount());
                instance->Configure(frameCount, channelCount);

                auto* processor = dynamic_cast<ProcessorNodeInstance*>(instance.get());
                AM_EXPECT_NOT(processor == nullptr);

                // Process multiple times to converge the smoothing
                const AudioBuffer* output = nullptr;
                for (int pass = 0; pass < 5; ++pass)
                {
                    instance->Reset();
                    output = processor->Process(&input);
                }

                if (output == nullptr)
                {
                    // At full occlusion, output could be nullptr (fully silent)
                    AM_EXPECT(tc.occlusion > 0.9f);
                    previousRMS = 0.0f;
                    continue;
                }

                AmReal32 outputRMS = 0.0f;
                for (AmUInt64 i = 0; i < frameCount; ++i)
                    outputRMS += (*output)[0][i] * (*output)[0][i];
                outputRMS = std::sqrt(outputRMS / static_cast<AmReal32>(frameCount));

                // RMS should decrease with increasing occlusion
                AM_EXPECT(outputRMS <= previousRMS);

                previousRMS = outputRMS;
            }
        }
    };

    AM_REGISTER_TEST(mixer_nodes, occlusion_node_reduces_gain_with_occlusion);
} // namespace SparkyStudios::Audio::Amplitude::Tests
