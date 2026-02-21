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
#include <Mixer/Nodes/AmbisonicPanningNode.h>

#include "LightEngineTestCase.h"
#include "NodeTestCase.h"
#include "TestRegistry.h"
#include "TestUtils.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(LightEngineTestCase, mixer_nodes, ambisonic_panning_node_produces_ambisonic_output)
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
            constexpr AmUInt16 channelCount = 1;
            constexpr AmUInt32 sampleRate = 48000;

            // Create listener at origin facing -Z
            ListenerInternalState listenerState;
            listenerState.SetLocation({ 0.0f, 0.0f, 0.0f });
            listenerState.SetOrientation(Orientation({ 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f }));
            listenerState.Update();
            Listener listener = InitTestListener(&listenerState);

            // Configure mock layer for HRTF spatialization
            SpatialMockLayer mockLayer;
            mockLayer.SetSampleRate(sampleRate);
            mockLayer.SetSpatialization(eSpatialization_HRTF);
            mockLayer.SetListener(listener);
            mockLayer.SetLocation({ 0.0f, 0.0f, -5.0f }); // Source ahead of listener

            SoundFormat format;
            format.SetAll(sampleRate, channelCount, 32, frameCount, channelCount * 4, eAudioSampleFormat_Float32);
            mockLayer.SetSoundFormat(format);

            AmbisonicPanningNode nodeFactory;
            auto instance = nodeFactory.CreateInstance();
            instance->Initialize(1, &mockLayer, nullptr, nodeFactory.GetParameterCount());
            instance->Configure(frameCount, channelCount);

            auto* processor = dynamic_cast<ProcessorNodeInstance*>(instance.get());
            AM_EXPECT_NOT(processor == nullptr);

            // ShouldSkip should be false for HRTF with valid listener
            AM_EXPECT_NOT(processor->ShouldSkip());

            // Generate mono input signal
            AudioBuffer input(frameCount, channelCount);
            for (AmUInt64 i = 0; i < frameCount; ++i)
                input[0][i] = std::sin(2.0f * AM_PI32 * 440.0f * static_cast<AmReal32>(i) / static_cast<AmReal32>(sampleRate));

            const AudioBuffer* output = processor->Process(&input);
            AM_EXPECT_NOT(output == nullptr);

            // Output should have ambisonic channel count: (order+1)^2 for 3D
            // BinauralHighQuality = 3, so order = 3, channels = 16
            const AmUInt16 outputChannels = output->GetChannelCount();
            AM_EXPECT(outputChannels > 1); // Must be multi-channel ambisonic

            // W channel (channel 0) should carry energy — it's the omnidirectional component
            AmReal32 wChannelRMS = 0.0f;
            for (AmUInt64 i = 0; i < frameCount; ++i)
                wChannelRMS += (*output)[0][i] * (*output)[0][i];
            wChannelRMS = std::sqrt(wChannelRMS / static_cast<AmReal32>(frameCount));

            AM_EXPECT(wChannelRMS > 0.01f);
        }
    };

    AM_REGISTER_TEST(mixer_nodes, ambisonic_panning_node_produces_ambisonic_output);
} // namespace SparkyStudios::Audio::Amplitude::Tests
