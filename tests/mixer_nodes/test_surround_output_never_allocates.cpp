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

#include "SpeakerPanningTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    // 7.1 sources on a 5.1 output with the pan and the spread moving every block: nothing goes through a pool.
    AM_TEST_CASE(SpeakerPanningTestCase, mixer_nodes, surround_output_never_allocates)
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

            SetUpListener();
            GetMockLayer().SetSpatialization(eSpatialization_Position);

            MockSound sound;
            GetMockLayer().SetSound(&sound);

            SpeakerPanningNode node;
            auto instance = node.CreateInstance();
            instance->Initialize(1, GetLayer(), nullptr, node.GetParameterCount());
            instance->SetPipelineOutputLayout(eChannelLayout_Surround_5_1);
            instance->Configure(512, 8);
            auto* processor = AsProcessor(instance);

            const AudioBuffer source = MakeNoise(512, 8);

            const AmUInt64 before = allocations();
            for (AmUInt32 block = 0; block < 1000; ++block)
            {
                GetMockLayer().SetLocation(LocationAt(static_cast<AmReal32>(block % 360)));
                sound.SetSpread(static_cast<AmReal32>((block * 7) % 361));
                processor->Reset();
                AM_UNUSED(processor->Process(&source));
            }
            AM_EXPECT_EQ(before, allocations());

            GetMockLayer().SetSound(nullptr);
#else
            AM_EXPECT(false);
#endif
        }
    };

    AM_REGISTER_TEST(mixer_nodes, surround_output_never_allocates);
} // namespace SparkyStudios::Audio::Amplitude::Tests
