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
    // Amplitude is X-right, Y-forward, Z-up: the directivity lobe of a source or of a listener has to point along its
    // forward axis (+Y), not along its right axis (+X).
    //
    // The directivity drives a one-pole low-pass in the occlusion node (coefficient = 1 - directivity / (lpf + 1)^4),
    // so it is observed through the RMS of a high tone: a larger directivity leaves a larger output.
    class OcclusionDirectivityTestCase : public LightEngineTestCase
    {
    public:
        void SetUp() override
        {
            LightEngineTestCase::SetUp();
            InitializeConfig(AM_OS_STRING("tests.config.amconfig"));
        }

    protected:
        struct Directivity
        {
            AmReal32 amount = 0.0f;
            AmReal32 sharpness = 1.0f;
        };

        // Renders one block through a fresh occlusion node and returns the RMS of the output. The listener sits at
        // the origin with a zero orientation (forward +Y). The source is an entity with its own directivity only when
        // entityDirectivity is given.
        AmReal32 Render(
            const AmVector3& sourceLocation, const Orientation& sourceOrientation, const Directivity& listenerDirectivity,
            const std::optional<Directivity>& entityDirectivity)
        {
            constexpr AmUInt64 frameCount = 1024;
            constexpr AmUInt16 channelCount = 1;
            constexpr AmUInt32 sampleRate = 48000;

            ListenerInternalState listenerState;
            listenerState.SetLocation({ 0.0f, 0.0f, 0.0f });
            listenerState.SetOrientation(Orientation::Zero());
            listenerState.SetDirectivity(listenerDirectivity.amount, listenerDirectivity.sharpness);
            listenerState.Update();
            const Listener listener = InitTestListener(&listenerState);

            EntityInternalState entityState;
            Entity entity;
            if (entityDirectivity.has_value())
            {
                entityState.SetLocation(sourceLocation);
                entityState.SetOrientation(sourceOrientation);
                entityState.SetDirectivity(entityDirectivity->amount, entityDirectivity->sharpness);
                entityState.Update();
                entity = InitTestEntity(&entityState);
            }

            // A high tone: the one-pole filter is nearly transparent at 440 Hz.
            AudioBuffer input(frameCount, channelCount);
            for (AmUInt64 i = 0; i < frameCount; ++i)
                input[0][i] = std::sin(2.0f * AM_PI32 * 12000.0f * static_cast<AmReal32>(i) / static_cast<AmReal32>(sampleRate));

            SpatialMockLayer layer;
            layer.SetSampleRate(sampleRate);

            SoundFormat format;
            format.SetAll(sampleRate, channelCount, 32, frameCount, channelCount * 4, eAudioSampleFormat_Float32);
            layer.SetSoundFormat(format);
            layer.SetOcclusion(0.5f);
            layer.SetListener(listener);
            layer.SetEntity(entity);
            layer.SetLocation(sourceLocation);
            layer.SetOrientation(sourceOrientation);
            layer.SetSpatialization(eSpatialization_Position);

            OcclusionNode nodeFactory;
            auto instance = nodeFactory.CreateInstance();
            instance->Initialize(1, &layer, nullptr, nodeFactory.GetParameterCount());
            instance->Configure(frameCount, channelCount);

            auto* processor = dynamic_cast<ProcessorNodeInstance*>(instance.get());
            AM_EXPECT_NOT(processor == nullptr);

            // Process multiple times to converge the occlusion smoothing.
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
        }
    };

    AM_TEST_CASE(OcclusionDirectivityTestCase, mixer_nodes, occlusion_node_entity_directivity_points_forward)
    {
    public:
        void Run() override
        {
            // The listener is straight ahead of the source (at +Y from it) when its orientation is zero.
            const AmVector3 source = { 0.0f, -5.0f, 0.0f };
            const Directivity none;
            const Directivity cardioid{ 0.5f, 1.0f };

            const AmReal32 facing = Render(source, Orientation(0.0f, 0.0f, 0.0f), none, cardioid);
            const AmReal32 away = Render(source, Orientation(AM_PI32, 0.0f, 0.0f), none, cardioid);
            const AmReal32 left = Render(source, Orientation(AM_PI32 / 2.0f, 0.0f, 0.0f), none, cardioid);
            const AmReal32 right = Render(source, Orientation(-AM_PI32 / 2.0f, 0.0f, 0.0f), none, cardioid);

            // On-axis is the loudest, off-axis is neutral, behind is the quietest.
            AM_EXPECT(facing > 1.5f * left);
            AM_EXPECT(left > 1e-3f);
            AM_EXPECT(facing > 1.5f * away);

            // The lobe is symmetric: a source turned to its left and to its right hears the listener alike.
            AM_EXPECT(std::abs(left - right) < 1e-4f);
        }
    };

    AM_TEST_CASE(OcclusionDirectivityTestCase, mixer_nodes, occlusion_node_listener_directivity_points_forward)
    {
    public:
        void Run() override
        {
            const Orientation zero = Orientation::Zero();
            const Directivity none;
            const Directivity cardioid{ 0.5f, 1.0f };

            // The listener faces +Y: a source is ahead, beside or behind it.
            const AmReal32 ahead = Render({ 0.0f, 5.0f, 0.0f }, zero, cardioid, std::nullopt);
            const AmReal32 right = Render({ 5.0f, 0.0f, 0.0f }, zero, cardioid, std::nullopt);
            const AmReal32 left = Render({ -5.0f, 0.0f, 0.0f }, zero, cardioid, std::nullopt);
            const AmReal32 behind = Render({ 0.0f, -5.0f, 0.0f }, zero, cardioid, std::nullopt);

            AM_EXPECT(ahead > 1.5f * right);
            AM_EXPECT(right > 1e-3f);
            AM_EXPECT(ahead > 1.5f * behind);
            AM_EXPECT(std::abs(right - left) < 1e-4f);

            // A source on the listener has no direction: it is on-axis, and the output stays finite.
            const AmReal32 coincident = Render({ 0.0f, 0.0f, 0.0f }, zero, cardioid, std::nullopt);
            AM_EXPECT(std::isfinite(coincident));
            AM_EXPECT(std::abs(coincident - ahead) < 1e-4f);

            // Without directivity, the position does not matter.
            AM_EXPECT(std::abs(Render({ 5.0f, 0.0f, 0.0f }, zero, none, std::nullopt) - Render({ 0.0f, 5.0f, 0.0f }, zero, none, std::nullopt)) < 1e-4f);
        }
    };

    AM_REGISTER_TEST(mixer_nodes, occlusion_node_entity_directivity_points_forward);
    AM_REGISTER_TEST(mixer_nodes, occlusion_node_listener_directivity_points_forward);
} // namespace SparkyStudios::Audio::Amplitude::Tests
