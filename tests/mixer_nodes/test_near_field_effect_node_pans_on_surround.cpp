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

#include <Mixer/Nodes/NearFieldEffectNode.h>

#include "SpeakerPanningTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(SpeakerPanningTestCase, mixer_nodes, near_field_effect_node_pans_on_surround)
    {
    public:
        void Run() override
        {
            constexpr AmUInt64 frames = 256;

            SetUpListener();
            GetMockLayer().SetSpatialization(eSpatialization_Position);
            GetMockLayer().SetLocation(LocationAt(70.0f, 0.3f));

            MockSound sound(1.0f);
            GetMockLayer().SetSound(&sound);

            NearFieldEffectNode node;
            auto instance = node.CreateInstance();
            instance->Initialize(1, GetLayer(), nullptr, node.GetParameterCount());
            instance->SetPipelineOutputLayout(eChannelLayout_Surround_5_1);
            instance->Configure(frames, 1);
            AM_EXPECT_EQ(6, instance->GetOutputChannelCount());

            AudioBuffer input(frames, 1);
            for (AmUInt64 i = 0; i < frames; ++i)
                input[0][i] = 0.5f;

            const AudioBuffer* output = AsProcessor(instance)->Process(&input);
            AM_EXPECT_NOT(output == nullptr);
            if (output != nullptr)
            {
                // 70 degrees left lies between L (30) and Ls (110): only those two speakers carry the boost.
                AmReal64 energy[6] = {};
                for (AmUInt16 c = 0; c < 6; ++c)
                    for (AmUInt64 i = 0; i < frames; ++i)
                        energy[c] += static_cast<AmReal64>((*output)[c][i]) * (*output)[c][i];

                AM_EXPECT(energy[0] > 0.0);
                AM_EXPECT(energy[4] > 0.0);
                for (const AmUInt16 c : { 1, 2, 3, 5 })
                    AM_EXPECT_EQ(0.0, energy[c]);
            }

            GetMockLayer().SetSound(nullptr);
        }
    };

    AM_REGISTER_TEST(mixer_nodes, near_field_effect_node_pans_on_surround);
} // namespace SparkyStudios::Audio::Amplitude::Tests
