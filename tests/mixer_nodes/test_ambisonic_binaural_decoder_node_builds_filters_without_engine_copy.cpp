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

#include <Core/Engine.h>
#include <Core/EngineInternalState.h>
#include <Mixer/Nodes/AmbisonicBinauralDecoderNode.h>
#include <Utils/Utils.h>

#include "LightEngineTestCase.h"
#include "NodeTestCase.h"
#include "TestRegistry.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(LightEngineTestCase, mixer_nodes, ambisonic_binaural_decoder_node_builds_filters_without_engine_copy)
    {
    public:
        void SetUp() override
        {
            LightEngineTestCase::SetUp();
            InitializeConfig(AM_OS_STRING("tests.config.amconfig"));
        }

        void Run() override
        {
            // Without the engine's copy, the node builds its own filters and still renders.
            auto state = amEngine->GetState();
            auto saved = state->binaural_filters;
            state->binaural_filters.reset();

            constexpr AmUInt64 frameCount = 512;
            constexpr AmUInt32 sampleRate = 48000;

            SpatialMockLayer mockLayer;
            mockLayer.SetSampleRate(sampleRate);

            AmbisonicBinauralDecoderNode nodeFactory;
            auto instance = nodeFactory.CreateInstance();
            const AmUInt16 ambiChannels = static_cast<AmUInt16>(OrderToComponents(3, true));
            instance->Initialize(1, &mockLayer, nullptr, nodeFactory.GetParameterCount());
            instance->Configure(frameCount, ambiChannels);

            auto* node = dynamic_cast<AmbisonicBinauralDecoderNodeInstance*>(instance.get());
            AM_EXPECT(node != nullptr);

            const auto& own = node->GetBinauralizer().GetFilters();
            AM_EXPECT(own != nullptr);
            AM_EXPECT(own.get() != saved.get());

            AudioBuffer input(frameCount, ambiChannels);
            for (AmUInt64 i = 0; i < frameCount; ++i)
                input[0][i] = std::sin(2.0f * AM_PI32 * 440.0f * static_cast<AmReal32>(i) / static_cast<AmReal32>(sampleRate));

            const AudioBuffer* output = node->Process(&input);
            AM_EXPECT(output != nullptr);

            AmReal32 energy = 0.0f;
            for (AmUInt64 i = 0; i < frameCount; ++i)
                energy += (*output)[0][i] * (*output)[0][i] + (*output)[1][i] * (*output)[1][i];
            AM_EXPECT(energy > 0.0f);

            // A node that took the engine's filters keeps them alive once the engine lets go.
            state->binaural_filters = saved;

            auto second = nodeFactory.CreateInstance();
            second->Initialize(1, &mockLayer, nullptr, nodeFactory.GetParameterCount());
            second->Configure(frameCount, ambiChannels);

            auto* secondNode = dynamic_cast<AmbisonicBinauralDecoderNodeInstance*>(second.get());
            AM_EXPECT(secondNode != nullptr);
            AM_EXPECT(secondNode->GetBinauralizer().GetFilters().get() == saved.get());

            std::weak_ptr<const BinauralDecodingFilters> weak = saved;
            saved.reset();
            state->binaural_filters.reset();

            AM_EXPECT(weak.lock() != nullptr);

            const AudioBuffer* secondOutput = secondNode->Process(&input);
            AM_EXPECT(secondOutput != nullptr);

            AmReal32 secondEnergy = 0.0f;
            for (AmUInt64 i = 0; i < frameCount; ++i)
                secondEnergy += (*secondOutput)[0][i] * (*secondOutput)[0][i] + (*secondOutput)[1][i] * (*secondOutput)[1][i];
            AM_EXPECT(secondEnergy > 0.0f);

            secondNode = nullptr;
            second.reset();
            AM_EXPECT(weak.expired());

            // Leave a normal engine behind for the tests that follow.
            state->binaural_filters = BinauralDecodingFilters::Build(3, true, *amEngine->GetHRIRSphere());
        }
    };

    AM_REGISTER_TEST(mixer_nodes, ambisonic_binaural_decoder_node_builds_filters_without_engine_copy);
} // namespace SparkyStudios::Audio::Amplitude::Tests
