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

#include <Mixer/Nodes/AmbisonicPanningNode.h>

#include "LightEngineTestCase.h"
#include "NodeTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(LightEngineTestCase, mixer_nodes, ambisonic_panning_node_skips_non_hrtf)
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

            // With Position spatialization (not HRTF), node should skip
            SpatialMockLayer mockLayer;
            mockLayer.SetSampleRate(sampleRate);
            mockLayer.SetSpatialization(eSpatialization_Position);

            SoundFormat format;
            format.SetAll(sampleRate, channelCount, 32, frameCount, channelCount * 4, eAudioSampleFormat_Float32);
            mockLayer.SetSoundFormat(format);

            AmbisonicPanningNode nodeFactory;
            auto instance = nodeFactory.CreateInstance();
            instance->Initialize(1, &mockLayer, nullptr, nodeFactory.GetParameterCount());
            instance->Configure(frameCount, channelCount);

            auto* processor = dynamic_cast<ProcessorNodeInstance*>(instance.get());
            AM_EXPECT_NOT(processor == nullptr);

            // ShouldSkip should return true for non-HRTF spatialization
            AM_EXPECT(processor->ShouldSkip());
        }
    };

    AM_REGISTER_TEST(mixer_nodes, ambisonic_panning_node_skips_non_hrtf);
} // namespace SparkyStudios::Audio::Amplitude::Tests
