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
    AM_TEST_CASE(LightEngineTestCase, mixer_nodes, ambisonic_binaural_decoder_nodes_share_engine_filters)
    {
    public:
        void SetUp() override
        {
            LightEngineTestCase::SetUp();
            InitializeConfig(AM_OS_STRING("tests.config.amconfig"));
        }

        void Run() override
        {
            // tests.config.amconfig is BinauralHighQuality: the engine prebuilt order-3 filters at init.
            const auto engineFilters = amEngine->GetState()->binaural_filters;
            AM_EXPECT(engineFilters != nullptr);
            AM_EXPECT(engineFilters->GetOrder() == 3);

            constexpr AmUInt64 frameCount = 512;
            constexpr AmUInt32 sampleRate = 48000;

            SpatialMockLayer mockLayer;
            mockLayer.SetSampleRate(sampleRate);

            AmbisonicBinauralDecoderNode nodeFactory;
            auto first = nodeFactory.CreateInstance();
            auto second = nodeFactory.CreateInstance();

            const AmUInt16 ambiChannels = static_cast<AmUInt16>(OrderToComponents(3, true));
            first->Initialize(1, &mockLayer, nullptr, nodeFactory.GetParameterCount());
            second->Initialize(2, &mockLayer, nullptr, nodeFactory.GetParameterCount());
            first->Configure(frameCount, ambiChannels);
            second->Configure(frameCount, ambiChannels);

            const auto* a = dynamic_cast<AmbisonicBinauralDecoderNodeInstance*>(first.get());
            const auto* b = dynamic_cast<AmbisonicBinauralDecoderNodeInstance*>(second.get());
            AM_EXPECT(a != nullptr && b != nullptr);
            AM_EXPECT(a->GetBinauralizer().GetFilters().get() == engineFilters.get());
            AM_EXPECT(b->GetBinauralizer().GetFilters().get() == engineFilters.get());
        }
    };

    AM_REGISTER_TEST(mixer_nodes, ambisonic_binaural_decoder_nodes_share_engine_filters);
} // namespace SparkyStudios::Audio::Amplitude::Tests
