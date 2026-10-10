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

#include "NodeTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(NodeTestCase, mixer_nodes, speaker_panning_node_skips_invalid_listener)
    {
    public:
        void Run() override
        {
            constexpr AmUInt64 frameCount = 256;
            constexpr AmUInt16 monoChannels = 1;

            // Default listener is invalid (nullptr state)
            // SpatialMockLayer default listener is default-constructed Listener{}

            auto instance = CreateAndConfigureNode<SpeakerPanningNode>(frameCount, monoChannels);
            auto* processor = AsProcessor(instance);
            AM_EXPECT_NOT(processor == nullptr);

            // A spatialized sound has nothing to pan against without a listener.
            GetMockLayer().SetSpatialization(eSpatialization_Position);
            AM_EXPECT(processor->ShouldSkip());

            // A 2D sound needs no listener.
            GetMockLayer().SetSpatialization(eSpatialization_None);
            AM_EXPECT_NOT(processor->ShouldSkip());
        }
    };

    AM_REGISTER_TEST(mixer_nodes, speaker_panning_node_skips_invalid_listener);
} // namespace SparkyStudios::Audio::Amplitude::Tests
