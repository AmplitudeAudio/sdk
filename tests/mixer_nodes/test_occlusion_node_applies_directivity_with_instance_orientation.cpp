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
    // The occlusion filter is driven by the directivity of the source towards the listener. For a separate-mode
    // instance with an orientation of its own, that directivity has to come from the instance's orientation, not
    // from the entity's.
    AM_TEST_CASE(LightEngineTestCase, mixer_nodes, occlusion_node_applies_directivity_with_instance_orientation)
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

            ListenerInternalState listenerState;
            listenerState.SetLocation({ 0.0f, 0.0f, 0.0f });
            listenerState.SetOrientation(Orientation({ 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f }));
            listenerState.Update();
            Listener listener = InitTestListener(&listenerState);

            // The source is diagonal to the listener in its horizontal plane, so a yaw of pi changes the directivity
            // whatever axis the directivity lobe follows: it is about 0.85 before the flip and 0.15 after it.
            const AmVector3 sourceLocation = { -5.0f, -5.0f, 0.0f };
            const Orientation entityOrientation = Orientation::Zero();
            const Orientation flippedOrientation(AM_PI32, 0.0f, 0.0f);

            // A half-directional source.
            EntityInternalState entityState;
            entityState.SetLocation(sourceLocation);
            entityState.SetOrientation(entityOrientation);
            entityState.SetDirectivity(0.5f, 1.0f);
            entityState.Update();
            Entity entity = InitTestEntity(&entityState);

            // A high tone: the filter the directivity selects is a one-pole low-pass, nearly transparent at 440 Hz.
            AudioBuffer input(frameCount, channelCount);
            for (AmUInt64 i = 0; i < frameCount; ++i)
                input[0][i] = std::sin(2.0f * AM_PI32 * 12000.0f * static_cast<AmReal32>(i) / static_cast<AmReal32>(sampleRate));

            const auto render = [&](const std::optional<Orientation>& instanceOrientation) -> AmReal32
            {
                SpatialMockLayer layer;
                layer.SetSampleRate(sampleRate);

                SoundFormat format;
                format.SetAll(sampleRate, channelCount, 32, frameCount, channelCount * 4, eAudioSampleFormat_Float32);
                layer.SetSoundFormat(format);
                layer.SetOcclusion(0.5f);
                layer.SetListener(listener);
                layer.SetEntity(entity);
                layer.SetLocation(sourceLocation);
                layer.SetSpatialization(eSpatialization_Position);

                if (instanceOrientation.has_value())
                    layer.SetOrientation(*instanceOrientation);

                OcclusionNode nodeFactory;
                auto instance = nodeFactory.CreateInstance();
                instance->Initialize(1, &layer, nullptr, nodeFactory.GetParameterCount());
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
                    return 0.0f;

                AmReal32 rms = 0.0f;
                for (AmUInt64 i = 0; i < frameCount; ++i)
                    rms += (*output)[0][i] * (*output)[0][i];

                return std::sqrt(rms / static_cast<AmReal32>(frameCount));
            };

            // No orientation on the instance follows the entity's.
            const AmReal32 followed = render(std::nullopt);
            const AmReal32 same = render(entityOrientation);
            const AmReal32 opposite = render(flippedOrientation);

            AM_EXPECT(std::abs(followed - same) < 1e-5f);
            AM_EXPECT(std::abs(followed - opposite) > 1e-3f);
        }
    };

    AM_REGISTER_TEST(mixer_nodes, occlusion_node_applies_directivity_with_instance_orientation);
} // namespace SparkyStudios::Audio::Amplitude::Tests
