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

#include <Mixer/Nodes/AmbisonicSpeakerDecoderNode.h>
#include <Utils/Utils.h>

#include "SpeakerPanningTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    // The speaker decoder runs on the audio thread: nothing goes through a pool while it renders.
    AM_TEST_CASE(EngineSpeakerPanningTestCase, mixer_nodes, ambisonic_speaker_decoder_node_never_allocates)
    {
    public:
        void Run() override
        {
#if !defined(AM_NO_MEMORY_STATS)
            const auto allocations = []()
            {
                AmUInt64 total = 0;
                for (AmUInt32 k = 0; k < eMemoryPoolKind_COUNT; ++k)
                    total += amMemory->GetStats(static_cast<eMemoryPoolKind>(k)).allocCount.load();
                return total;
            };

            const AmUInt32 order = AM_MAX(static_cast<AmUInt32>(amEngine->GetPanningMode()), 1u);
            const AmUInt16 components = static_cast<AmUInt16>(OrderToComponents(order, true));

            AmbisonicSpeakerDecoderNode node;
            auto instance = node.CreateInstance();
            instance->Initialize(1, GetLayer(), nullptr, node.GetParameterCount());
            instance->SetPipelineOutputLayout(eChannelLayout_Surround_7_1);
            instance->Configure(512, components);
            auto* processor = AsProcessor(instance);

            const AudioBuffer source = MakeNoise(512, components);

            const AmUInt64 before = allocations();
            for (AmUInt32 block = 0; block < 1000; ++block)
            {
                processor->Reset();
                AM_UNUSED(processor->Process(&source));
            }
            AM_EXPECT_EQ(before, allocations());
#else
            AM_EXPECT(false);
#endif
        }
    };

    AM_REGISTER_TEST(mixer_nodes, ambisonic_speaker_decoder_node_never_allocates);
} // namespace SparkyStudios::Audio::Amplitude::Tests
