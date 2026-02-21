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

#include <Mixer/Nodes/ObstructionNode.h>

#include "LightEngineTestCase.h"
#include "NodeTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(LightEngineTestCase, mixer_nodes, obstruction_node_applies_lpf_at_full)
    {
    public:
        void SetUp() override
        {
            LightEngineTestCase::SetUp();
            InitializeConfig(AM_OS_STRING("tests.config.amconfig"));
        }

        void Run() override
        {
            constexpr AmUInt64 frameCount = 4096;
            constexpr AmUInt16 channelCount = 1;
            constexpr AmUInt32 sampleRate = 48000;

            // Create mock layer with full obstruction
            SpatialMockLayer mockLayer;
            mockLayer.SetSampleRate(sampleRate);

            SoundFormat format;
            format.SetAll(sampleRate, channelCount, 32, frameCount, channelCount * 4, eAudioSampleFormat_Float32);
            mockLayer.SetSoundFormat(format);
            mockLayer.SetObstruction(1.0f);

            // Create and configure node
            ObstructionNode nodeFactory;
            auto instance = nodeFactory.CreateInstance();
            instance->Initialize(1, &mockLayer, nullptr, nodeFactory.GetParameterCount());
            instance->Configure(frameCount, channelCount);

            auto* processor = dynamic_cast<ProcessorNodeInstance*>(instance.get());
            AM_EXPECT_NOT(processor == nullptr);

            // Generate high-frequency input (should be attenuated by LPF)
            AudioBuffer input(frameCount, channelCount);
            for (AmUInt64 i = 0; i < frameCount; ++i)
                input[0][i] = std::sin(2.0f * AM_PI32 * 5000.0f * static_cast<AmReal32>(i) / static_cast<AmReal32>(sampleRate));

            // Process multiple times to let the smoothing converge
            // (obstruction uses Lerp with 0.75 smoothing coefficient)
            const AudioBuffer* output = nullptr;
            for (int pass = 0; pass < 5; ++pass)
            {
                instance->Reset();
                output = processor->Process(&input);
            }

            AM_EXPECT_NOT(output == nullptr);

            // With obstruction=1 and test config:
            //   LPF curve: lpf=1 → full LPF applied (MonoPole filter at coefficient ~1.0)
            //   Gain curve: gain=1 → no gain reduction
            // The LPF should attenuate the high-frequency signal, reducing its energy
            AmReal32 inputRMS = 0.0f;
            AmReal32 outputRMS = 0.0f;
            for (AmUInt64 i = frameCount / 2; i < frameCount; ++i)
            {
                inputRMS += input[0][i] * input[0][i];
                outputRMS += (*output)[0][i] * (*output)[0][i];
            }
            inputRMS = std::sqrt(inputRMS / static_cast<AmReal32>(frameCount / 2));
            outputRMS = std::sqrt(outputRMS / static_cast<AmReal32>(frameCount / 2));

            // Output should have lower RMS than input due to LPF on high-frequency signal
            AM_EXPECT(outputRMS < inputRMS);

            // Output should not be completely silent (gain curve keeps gain at 1.0)
            AM_EXPECT(outputRMS > 0.001f);
        }
    };

    AM_REGISTER_TEST(mixer_nodes, obstruction_node_applies_lpf_at_full);
} // namespace SparkyStudios::Audio::Amplitude::Tests
