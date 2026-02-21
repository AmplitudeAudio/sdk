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
#include <Mixer/Nodes/AmbisonicRotatorNode.h>

#include "LightEngineTestCase.h"
#include "NodeTestCase.h"
#include "TestRegistry.h"
#include "TestUtils.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(LightEngineTestCase, mixer_nodes, ambisonic_rotator_node_rotates_soundfield)
    {
    public:
        void SetUp() override
        {
            LightEngineTestCase::SetUp();
            InitializeConfig(AM_OS_STRING("tests.config.amconfig"));
        }

        void Run() override
        {
            constexpr AmUInt64 frameCount = 512;
            constexpr AmUInt32 sampleRate = 48000;

            // Create a listener facing -Z (default forward)
            ListenerInternalState listenerState;
            listenerState.SetLocation({ 0.0f, 0.0f, 0.0f });
            listenerState.SetOrientation(Orientation({ 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f }));
            listenerState.Update();
            Listener listener = InitTestListener(&listenerState);

            SpatialMockLayer mockLayer;
            mockLayer.SetSampleRate(sampleRate);
            mockLayer.SetSpatialization(eSpatialization_HRTF);
            mockLayer.SetListener(listener);

            SoundFormat format;
            format.SetAll(sampleRate, 1, 32, frameCount, 4, eAudioSampleFormat_Float32);
            mockLayer.SetSoundFormat(format);

            AmbisonicRotatorNode nodeFactory;
            auto instance = nodeFactory.CreateInstance();

            // Get the output channel count (ambisonic components)
            const AmUInt16 ambiChannels = instance->GetOutputChannelCount();
            AM_EXPECT(ambiChannels > 1);

            instance->Initialize(1, &mockLayer, nullptr, nodeFactory.GetParameterCount());
            instance->Configure(frameCount, ambiChannels);

            auto* processor = dynamic_cast<ProcessorNodeInstance*>(instance.get());
            AM_EXPECT_NOT(processor == nullptr);
            AM_EXPECT_NOT(processor->ShouldSkip());

            // Create ambisonic input with energy in channel 0 (W) and channel 1 (Y)
            AudioBuffer input(frameCount, ambiChannels);
            for (AmUInt64 i = 0; i < frameCount; ++i)
            {
                AmReal32 sample = std::sin(2.0f * AM_PI32 * 440.0f * static_cast<AmReal32>(i) / static_cast<AmReal32>(sampleRate));
                input[0][i] = sample;       // W channel (omnidirectional)
                if (ambiChannels > 1)
                    input[1][i] = sample;   // Y channel (left-right)
            }

            const AudioBuffer* output = processor->Process(&input);
            AM_EXPECT_NOT(output == nullptr);

            // Output should have same channel count as input (ambisonic)
            AM_EXPECT((output->GetChannelCount()) == (ambiChannels));

            // W channel (channel 0) should be preserved by rotation (omnidirectional)
            AmReal32 wInputRMS = 0.0f;
            AmReal32 wOutputRMS = 0.0f;
            for (AmUInt64 i = 0; i < frameCount; ++i)
            {
                wInputRMS += input[0][i] * input[0][i];
                wOutputRMS += (*output)[0][i] * (*output)[0][i];
            }
            wInputRMS = std::sqrt(wInputRMS / static_cast<AmReal32>(frameCount));
            wOutputRMS = std::sqrt(wOutputRMS / static_cast<AmReal32>(frameCount));

            // W channel energy should be approximately preserved (rotation doesn't change W)
            AM_EXPECT(std::abs(wOutputRMS - wInputRMS) < 0.05f);

            // Total energy across all channels should be approximately preserved
            AmReal32 totalInputEnergy = 0.0f;
            AmReal32 totalOutputEnergy = 0.0f;
            for (AmUInt16 ch = 0; ch < ambiChannels; ++ch)
            {
                for (AmUInt64 i = 0; i < frameCount; ++i)
                {
                    totalInputEnergy += input[ch][i] * input[ch][i];
                    totalOutputEnergy += (*output)[ch][i] * (*output)[ch][i];
                }
            }

            // Energy should be conserved (within tolerance for numerical precision)
            AmReal32 energyRatio = totalOutputEnergy / totalInputEnergy;
            AM_EXPECT(energyRatio > 0.9f);
            AM_EXPECT(energyRatio < 1.1f);
        }
    };

    AM_REGISTER_TEST(mixer_nodes, ambisonic_rotator_node_rotates_soundfield);
} // namespace SparkyStudios::Audio::Amplitude::Tests
