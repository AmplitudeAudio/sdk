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

#include <cmath>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Mixer/Pipeline.h>

#include "EngineTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(EngineTestCase, mixer_pipeline, mono_policy_node_gets_fold_down)
    {
    public:
        void Run() override
        {
            constexpr AmUInt64 frames = 128;
            AmplimixLayerImpl layer;

            PipelineImpl pipeline;
            pipeline.LoadDefinitionFromFile(
                _fileSystem->OpenFile(
                    _fileSystem->Join({ AM_OS_STRING("pipelines"), AM_OS_STRING("tests.mono_probe.ampipeline") }), eFileOpenMode_Read),
                nullptr);

            auto instance = pipeline.CreateInstance(&layer);
            AM_EXPECT_NOT(instance == nullptr);
            if (instance == nullptr)
                return;

            // A stereo source into a node that never declared a policy: it gets the ITU mono fold-down.
            AM_EXPECT(instance->Configure(frames, 2, frames, 1));

            AudioBuffer input(frames, 2);
            for (AmUInt64 i = 0; i < frames; ++i)
            {
                input[0][i] = 1.0f;
                input[1][i] = 0.5f;
            }

            AudioBuffer output(frames, 1);
            instance->Execute(input, output);

            const auto* probe = dynamic_cast<ChannelProbeNodeInstance*>(instance->GetNode(2).get());
            AM_EXPECT_NOT(probe == nullptr);
            if (probe == nullptr)
                return;

            constexpr AmReal32 expected = 0.70710678f * 1.5f;
            AM_EXPECT_EQ(1, probe->receivedChannels);
            AM_EXPECT(std::abs(probe->firstSample - expected) < 1e-6f);
            AM_EXPECT(std::abs(output[0][frames - 1] - expected) < 1e-6f);
        }
    };

    AM_REGISTER_TEST(mixer_pipeline, mono_policy_node_gets_fold_down);
} // namespace SparkyStudios::Audio::Amplitude::Tests
