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

#include <Mixer/Pipeline.h>

#include "EngineTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    // A post-panning plugin node keeps receiving every channel, as it did before channel policies existed.
    AM_TEST_CASE(EngineTestCase, mixer_pipeline, mono_policy_node_after_custom_keeps_channels)
    {
    public:
        void Run() override
        {
            constexpr AmUInt64 frames = 128;
            AmplimixLayerImpl layer;

            PipelineImpl pipeline;
            pipeline.LoadDefinitionFromFile(
                _fileSystem->OpenFile(
                    _fileSystem->Join({ AM_OS_STRING("pipelines"), AM_OS_STRING("tests.custom_then_mono_probe.ampipeline") }),
                    eFileOpenMode_Read),
                nullptr);

            auto instance = pipeline.CreateInstance(&layer);
            AM_EXPECT_NOT(instance == nullptr);
            if (instance == nullptr)
                return;

            AM_EXPECT(instance->Configure(frames, 2, frames, 2));

            AudioBuffer input(frames, 2);
            for (AmUInt64 i = 0; i < frames; ++i)
            {
                input[0][i] = 1.0f;
                input[1][i] = 0.5f;
            }

            AudioBuffer output(frames, 2);
            instance->Execute(input, output);

            const auto* probe = dynamic_cast<ChannelProbeNodeInstance*>(instance->GetNode(3).get());
            AM_EXPECT_NOT(probe == nullptr);
            if (probe == nullptr)
                return;

            AM_EXPECT_EQ(2, probe->receivedChannels);
            AM_EXPECT_EQ(1.0f, probe->firstSample);
            AM_EXPECT_EQ(0.5f, output[1][0]);
        }
    };

    AM_REGISTER_TEST(mixer_pipeline, mono_policy_node_after_custom_keeps_channels);
} // namespace SparkyStudios::Audio::Amplitude::Tests
