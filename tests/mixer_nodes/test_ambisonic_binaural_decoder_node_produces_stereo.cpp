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

#include <Mixer/Nodes/AmbisonicBinauralDecoderNode.h>
#include <Utils/Utils.h>

#include "LightEngineTestCase.h"
#include "NodeTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(LightEngineTestCase, mixer_nodes, ambisonic_binaural_decoder_node_produces_stereo)
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

            SpatialMockLayer mockLayer;
            mockLayer.SetSampleRate(sampleRate);

            SoundFormat format;
            format.SetAll(sampleRate, 1, 32, frameCount, 4, eAudioSampleFormat_Float32);
            mockLayer.SetSoundFormat(format);

            AmbisonicBinauralDecoderNode nodeFactory;
            auto instance = nodeFactory.CreateInstance();

            // Output should be stereo (2 channels) — binaural decoder
            AM_EXPECT((instance->GetOutputChannelCount()) == (kAmStereoChannelCount));

            // Compute ambisonic channel count from panning mode
            const ePanningMode mode = amEngine->GetPanningMode();
            const AmUInt32 order = AM_MAX(static_cast<AmUInt32>(mode), 1u);
            const AmUInt16 ambiChannels = static_cast<AmUInt16>(OrderToComponents(order, true));

            instance->Initialize(1, &mockLayer, nullptr, nodeFactory.GetParameterCount());
            instance->Configure(frameCount, ambiChannels);

            auto* processor = dynamic_cast<ProcessorNodeInstance*>(instance.get());
            AM_EXPECT_NOT(processor == nullptr);

            // Create ambisonic input with energy in W channel
            AudioBuffer input(frameCount, ambiChannels);
            for (AmUInt64 i = 0; i < frameCount; ++i)
                input[0][i] = std::sin(2.0f * AM_PI32 * 440.0f * static_cast<AmReal32>(i) / static_cast<AmReal32>(sampleRate));

            const AudioBuffer* output = processor->Process(&input);
            AM_EXPECT_NOT(output == nullptr);

            // Output must be stereo
            AM_EXPECT((output->GetChannelCount()) == (kAmStereoChannelCount));

            // Both channels should have energy (W channel decodes to both ears)
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

    AM_REGISTER_TEST(mixer_nodes, ambisonic_binaural_decoder_node_produces_stereo);
} // namespace SparkyStudios::Audio::Amplitude::Tests
