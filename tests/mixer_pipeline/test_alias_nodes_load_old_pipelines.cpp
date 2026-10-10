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

#include <Mixer/Nodes/SpeakerPanningNode.h>
#include <Mixer/Pipeline.h>

#include "EngineTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    // stereo.json still names StereoPanning (node 4) and StereoMixer (node 8): both resolve to the new nodes.
    AM_TEST_CASE(EngineTestCase, mixer_pipeline, alias_nodes_load_old_pipelines)
    {
    public:
        void Run() override
        {
            AmplimixLayerImpl layer;

            PipelineImpl pipeline;
            pipeline.LoadDefinitionFromFile(
                _fileSystem->OpenFile(_fileSystem->Join({ AM_OS_STRING("pipelines"), AM_OS_STRING("stereo.ampipeline") }), eFileOpenMode_Read),
                nullptr);

            auto instance = pipeline.CreateInstance(&layer);
            AM_EXPECT_NOT(instance == nullptr);
            if (instance == nullptr)
                return;

            AM_EXPECT_NOT(dynamic_cast<SpeakerPanningNodeInstance*>(instance->GetNode(4).get()) == nullptr);
            AM_EXPECT_NOT(dynamic_cast<MixerNodeInstance*>(instance->GetNode(8).get()) == nullptr);

            for (const char* name : { "StereoPanning", "StereoMixer", "SpeakerPanning", "Mixer" })
            {
                const auto node = Node::Find(name);
                AM_EXPECT_NOT(node == nullptr);
                if (node != nullptr)
                    AM_EXPECT_EQ(eNodeChannelPolicy_Custom, node->GetChannelPolicy());
            }
        }
    };

    AM_REGISTER_TEST(mixer_pipeline, alias_nodes_load_old_pipelines);
} // namespace SparkyStudios::Audio::Amplitude::Tests
