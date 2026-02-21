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
#include <Mixer/Nodes/NearFieldEffectNode.h>

#include "LightEngineTestCase.h"
#include "NodeTestCase.h"
#include "TestRegistry.h"
#include "TestUtils.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(LightEngineTestCase, mixer_nodes, near_field_effect_node_stereo_output_near_source)
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

            // Listener at origin
            ListenerInternalState listenerState;
            listenerState.SetLocation({ 0.0f, 0.0f, 0.0f });
            listenerState.SetOrientation(Orientation({ 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f }));
            listenerState.Update();
            Listener listener = InitTestListener(&listenerState);

            // Mock sound with near-field gain enabled
            MockSound mockSound(1.0f);

            // Source very close (0.3 meters — within kNearFieldMaxDistance of 1m)
            SpatialMockLayer mockLayer;
            mockLayer.SetSampleRate(sampleRate);
            mockLayer.SetSpatialization(eSpatialization_Position);
            mockLayer.SetListener(listener);
            mockLayer.SetLocation({ 0.0f, 0.0f, -0.3f }); // Close, directly ahead
            mockLayer.SetSound(&mockSound);

            SoundFormat format;
            format.SetAll(sampleRate, channelCount, 32, frameCount, channelCount * 4, eAudioSampleFormat_Float32);
            mockLayer.SetSoundFormat(format);

            NearFieldEffectNode nodeFactory;
            auto instance = nodeFactory.CreateInstance();
            instance->Initialize(1, &mockLayer, nullptr, nodeFactory.GetParameterCount());
            instance->Configure(frameCount, channelCount);

            auto* processor = dynamic_cast<ProcessorNodeInstance*>(instance.get());
            AM_EXPECT_NOT(processor == nullptr);

            // Output should be stereo
            AM_EXPECT((instance->GetOutputChannelCount()) == (kAmStereoChannelCount));

            // Generate mono input
            AudioBuffer input(frameCount, channelCount);
            for (AmUInt64 i = 0; i < frameCount; ++i)
                input[0][i] = std::sin(2.0f * AM_PI32 * 440.0f * static_cast<AmReal32>(i) / static_cast<AmReal32>(sampleRate));

            // Process multiple times for gain smoothing to converge
            const AudioBuffer* output = nullptr;
            for (int pass = 0; pass < 5; ++pass)
            {
                instance->Reset();
                output = processor->Process(&input);
            }

            // Near source should produce non-null stereo output
            AM_EXPECT_NOT(output == nullptr);
            AM_EXPECT((output->GetChannelCount()) == (kAmStereoChannelCount));

            // Both channels should have energy
            AmReal32 leftRMS = 0.0f;
            AmReal32 rightRMS = 0.0f;
            for (AmUInt64 i = 0; i < frameCount; ++i)
            {
                leftRMS += (*output)[0][i] * (*output)[0][i];
                rightRMS += (*output)[1][i] * (*output)[1][i];
            }
            leftRMS = std::sqrt(leftRMS / static_cast<AmReal32>(frameCount));
            rightRMS = std::sqrt(rightRMS / static_cast<AmReal32>(frameCount));

            AM_EXPECT(leftRMS > 0.001f);
            AM_EXPECT(rightRMS > 0.001f);
        }
    };

    AM_REGISTER_TEST(mixer_nodes, near_field_effect_node_stereo_output_near_source);
} // namespace SparkyStudios::Audio::Amplitude::Tests
