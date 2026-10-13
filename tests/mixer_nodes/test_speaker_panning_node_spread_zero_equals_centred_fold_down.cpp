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

#include <DSP/ChannelMap.h>
#include <DSP/SpreadEmitters.h>

#include "SpeakerPanningTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(SpeakerPanningTestCase, mixer_nodes, speaker_panning_node_spread_zero_equals_centred_fold_down)
    {
    public:
        void Run() override
        {
            SetUpListener();
            GetMockLayer().SetSpatialization(eSpatialization_Position);
            GetMockLayer().SetLocation(LocationAt(40.0f));

            MockSound sound;
            sound.SetSpread(0.0f);
            GetMockLayer().SetSound(&sound);

            for (const AmUInt16 channels : { 2, 4, 6, 8 })
            {
                const ChannelLayout& layout = *ChannelLayout::FromChannelCount(channels);
                const AudioBuffer source = MakeNoise(256, channels);

                AudioBuffer mono(256, 1);
                ApplyChannelMap(ChannelLayout::GetFoldDownMatrix(layout, ChannelLayout::Get(eChannelLayout_Mono)), source, mono, 256);

                // Outputs without an LFE: the source LFE has its own path (lfe_bypasses_panning).
                for (const eChannelLayout output : { eChannelLayout_Stereo, eChannelLayout_Quad })
                    AM_EXPECT(Near(Render(source, output), Render(mono, output), 1e-6f));
            }

            GetMockLayer().SetSound(nullptr);
        }
    };

    AM_REGISTER_TEST(mixer_nodes, speaker_panning_node_spread_zero_equals_centred_fold_down);
} // namespace SparkyStudios::Audio::Amplitude::Tests
