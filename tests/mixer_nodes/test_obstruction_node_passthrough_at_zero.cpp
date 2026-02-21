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
    AM_TEST_CASE(LightEngineTestCase, mixer_nodes, obstruction_node_passthrough_at_zero)
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

            // Create mock layer with zero obstruction
            SpatialMockLayer mockLayer;
            mockLayer.SetSampleRate(sampleRate);

            SoundFormat format;
            format.SetAll(sampleRate, channelCount, 32, frameCount, channelCount * 4, eAudioSampleFormat_Float32);
            mockLayer.SetSoundFormat(format);
            mockLayer.SetObstruction(0.0f);

            // Create and configure node
            ObstructionNode nodeFactory;
            auto instance = nodeFactory.CreateInstance();
            instance->Initialize(1, &mockLayer, nullptr, nodeFactory.GetParameterCount());
            instance->Configure(frameCount, channelCount);

            auto* processor = dynamic_cast<ProcessorNodeInstance*>(instance.get());
            AM_EXPECT_NOT(processor == nullptr);

            // Generate input signal
            AudioBuffer input(frameCount, channelCount);
            for (AmUInt64 i = 0; i < frameCount; ++i)
                input[0][i] = std::sin(2.0f * AM_PI32 * 440.0f * static_cast<AmReal32>(i) / static_cast<AmReal32>(sampleRate));

            const AudioBuffer* output = processor->Process(&input);
            AM_EXPECT_NOT(output == nullptr);

            // With obstruction=0, the test config has:
            //   LPF curve: (0,0)→(1,1), so lpf=0 at obstruction=0 → no filter applied
            //   Gain curve: (0,1)→(1,1), so gain=1 at obstruction=0 → no gain change
            // Output should match input closely
            AmReal32 maxDiff = 0.0f;
            for (AmUInt64 i = 0; i < frameCount; ++i)
                maxDiff = std::max(maxDiff, std::abs((*output)[0][i] - input[0][i]));

            AM_EXPECT(maxDiff < 0.01f);
        }
    };

    AM_REGISTER_TEST(mixer_nodes, obstruction_node_passthrough_at_zero);
} // namespace SparkyStudios::Audio::Amplitude::Tests
