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
    AM_TEST_CASE(LightEngineTestCase, mixer_nodes, near_field_effect_node_keeps_state_across_blocks)
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

            // Source at 0.5 m (inside the 1 m near-field range), HRTF mode engages the delay path
            SpatialMockLayer mockLayer;
            mockLayer.SetSampleRate(sampleRate);
            mockLayer.SetSpatialization(eSpatialization_HRTF);
            mockLayer.SetListener(listener);
            mockLayer.SetLocation({ 0.0f, 0.0f, -0.5f });
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

            // Block 1: impulse at the last sample — charges the biquad filter and delay state
            // right before the block boundary, so the tail is still strong in the next block.
            AudioBuffer impulse(frameCount, channelCount);
            impulse[0][frameCount - 1] = 1.0f;
            const AudioBuffer* out1 = processor->Process(&impulse);
            AM_EXPECT_NOT(out1 == nullptr);

            // Block 2: silence. A persistent processor carries the filter/delay tail into this
            // block; a per-block-reconstructed processor would output exactly zero.
            AudioBuffer silence(frameCount, channelCount);
            const AudioBuffer* out2 = processor->Process(&silence);
            AM_EXPECT_NOT(out2 == nullptr);

            AmReal32 tailPeak = 0.0f;
            for (AmUInt64 ch = 0; ch < out2->GetChannelCount(); ++ch)
                for (AmUInt64 i = 0; i < frameCount; ++i)
                    tailPeak = std::max(tailPeak, std::abs((*out2)[ch][i]));

            AM_EXPECT(tailPeak > 1e-6f);
        }
    };

    AM_REGISTER_TEST(mixer_nodes, near_field_effect_node_keeps_state_across_blocks);
} // namespace SparkyStudios::Audio::Amplitude::Tests
