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

#include <Mixer/Nodes/HardClipNode.h>
#include <Mixer/Nodes/ObstructionNode.h>

#include "LightEngineTestCase.h"
#include "NodeTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    namespace
    {
        std::shared_ptr<NodeInstance> CreateConfigured(const Node& node, AmplimixLayer* layer, AmUInt64 frames, AmUInt16 channels)
        {
            auto instance = node.CreateInstance();
            instance->Initialize(1, layer, nullptr, node.GetParameterCount());
            instance->Configure(frames, channels);
            return instance;
        }
    } // namespace

    AM_TEST_CASE(LightEngineTestCase, mixer_nodes, per_channel_nodes_keep_every_channel)
    {
    public:
        void SetUp() override
        {
            LightEngineTestCase::SetUp();
            InitializeConfig(AM_OS_STRING("tests.config.amconfig"));
        }

        void Run() override
        {
            constexpr AmUInt64 frames = 64;
            constexpr AmUInt16 channels = 6;

            for (const char* name :
                 { "Attenuation", "Occlusion", "Obstruction", "EnvironmentEffect", "HardClip", "RoundoffClip", "Clamp", "Limiter" })
            {
                const auto node = Node::Find(name);
                AM_EXPECT_NOT(node == nullptr);
                if (node != nullptr)
                    AM_EXPECT_EQ(eNodeChannelPolicy_PerChannel, node->GetChannelPolicy());
            }
            for (const char* name : { "StereoMixer", "AmbisonicMixer", "AmbisonicRotator", "AmbisonicBinauralDecoder" })
            {
                const auto node = Node::Find(name);
                AM_EXPECT_NOT(node == nullptr);
                if (node != nullptr)
                    AM_EXPECT_EQ(eNodeChannelPolicy_Custom, node->GetChannelPolicy());
            }
            for (const char* name : { "NearFieldEffect", "Reflections", "Reverb" })
            {
                const auto node = Node::Find(name);
                AM_EXPECT_NOT(node == nullptr);
                if (node != nullptr)
                    AM_EXPECT_EQ(eNodeChannelPolicy_Mono, node->GetChannelPolicy());
            }

            SpatialMockLayer layer;
            layer.SetSampleRate(48000);
            SoundFormat format;
            format.SetAll(48000, channels, 32, frames, channels * 4, eAudioSampleFormat_Float32);
            layer.SetSoundFormat(format);
            layer.SetObstruction(1.0f);

            // A 5.1 buffer with an impulse on L only: every other channel stays silent through the per-channel filters.
            AudioBuffer input(frames, channels);
            input[0][0] = 0.5f;

            ObstructionNode obstructionFactory;
            auto obstruction = CreateConfigured(obstructionFactory, &layer, frames, channels);
            auto* obstructionProcessor = dynamic_cast<ProcessorNodeInstance*>(obstruction.get());
            AM_EXPECT_NOT(obstructionProcessor == nullptr);
            if (obstructionProcessor == nullptr)
                return;

            const AudioBuffer* obstructed = obstructionProcessor->Process(&input);
            AM_EXPECT_NOT(obstructed == nullptr);
            if (obstructed == nullptr)
                return;

            AM_EXPECT_EQ(channels, obstructed->GetChannelCount());
            for (AmUInt16 c = 1; c < channels; ++c)
                for (AmUInt64 i = 0; i < frames; ++i)
                    AM_EXPECT_EQ(0.0f, (*obstructed)[c][i]);

            HardClipNode clipFactory;
            auto clip = CreateConfigured(clipFactory, &layer, frames, channels);
            auto* clipProcessor = dynamic_cast<ProcessorNodeInstance*>(clip.get());
            AM_EXPECT_NOT(clipProcessor == nullptr);
            if (clipProcessor == nullptr)
                return;

            const AudioBuffer* clipped = clipProcessor->Process(&input);
            AM_EXPECT_NOT(clipped == nullptr);
            if (clipped == nullptr)
                return;

            AM_EXPECT_EQ(channels, clipped->GetChannelCount());
            AM_EXPECT_EQ(0.5f, (*clipped)[0][0]);
        }
    };

    AM_REGISTER_TEST(mixer_nodes, per_channel_nodes_keep_every_channel);
} // namespace SparkyStudios::Audio::Amplitude::Tests
