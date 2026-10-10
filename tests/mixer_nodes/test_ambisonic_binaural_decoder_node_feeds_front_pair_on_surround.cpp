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
    AM_TEST_CASE(LightEngineTestCase, mixer_nodes, ambisonic_binaural_decoder_node_feeds_front_pair_on_surround)
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

            const ePanningMode mode = amEngine->GetPanningMode();
            const AmUInt32 order = AM_MAX(static_cast<AmUInt32>(mode), 1u);
            const AmUInt16 ambiChannels = static_cast<AmUInt16>(OrderToComponents(order, true));

            AudioBuffer input(frameCount, ambiChannels);
            for (AmUInt64 i = 0; i < frameCount; ++i)
                input[0][i] = std::sin(2.0f * AM_PI32 * 440.0f * static_cast<AmReal32>(i) / static_cast<AmReal32>(sampleRate));

            const auto render = [&](eChannelLayout layout)
            {
                AmbisonicBinauralDecoderNode nodeFactory;
                auto instance = nodeFactory.CreateInstance();
                instance->Initialize(1, &mockLayer, nullptr, nodeFactory.GetParameterCount());
                instance->SetPipelineOutputLayout(layout);
                instance->Configure(frameCount, ambiChannels);
                AM_EXPECT_EQ(ChannelLayout::Get(layout).GetChannelCount(), instance->GetOutputChannelCount());

                const AudioBuffer* output = dynamic_cast<ProcessorNodeInstance*>(instance.get())->Process(&input);
                AudioBuffer copy(frameCount, ChannelLayout::Get(layout).GetChannelCount());
                AM_EXPECT_NOT(output == nullptr);
                if (output != nullptr)
                    AudioBuffer::Copy(*output, 0, copy, 0, frameCount);

                return copy;
            };

            const AudioBuffer stereo = render(eChannelLayout_Stereo);
            const AudioBuffer surround = render(eChannelLayout_Surround_5_1);

            // L and R carry the binaural render exactly; every other speaker is silent.
            for (AmUInt64 i = 0; i < frameCount; ++i)
            {
                AM_EXPECT_EQ(stereo[0][i], surround[0][i]);
                AM_EXPECT_EQ(stereo[1][i], surround[1][i]);
                for (AmUInt16 c = 2; c < 6; ++c)
                    AM_EXPECT_EQ(0.0f, surround[c][i]);
            }
        }
    };

    AM_REGISTER_TEST(mixer_nodes, ambisonic_binaural_decoder_node_feeds_front_pair_on_surround);
} // namespace SparkyStudios::Audio::Amplitude::Tests
