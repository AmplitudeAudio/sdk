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
            const auto saved = state->binaural_filters;
            std::weak_ptr<const BinauralDecodingFilters> engineCopy = saved;
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

            // Filters outlive their engine owner while a user holds them.
            state->binaural_filters = saved;
            AM_EXPECT(engineCopy.lock() != nullptr);
        }
    };

    AM_REGISTER_TEST(mixer_nodes, ambisonic_binaural_decoder_node_builds_filters_without_engine_copy);
} // namespace SparkyStudios::Audio::Amplitude::Tests
