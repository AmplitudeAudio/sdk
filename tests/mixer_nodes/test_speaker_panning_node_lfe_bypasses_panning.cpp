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
    AM_TEST_CASE(SpeakerPanningTestCase, mixer_nodes, speaker_panning_node_lfe_bypasses_panning)
    {
    public:
        void Run() override
        {
            SetUpListener();
            GetMockLayer().SetSpatialization(eSpatialization_Position);
            GetMockLayer().SetLocation(LocationAt(90.0f));

            // Content on the 5.1 LFE only.
            AudioBuffer source(256, 6);
            const AudioBuffer noise = MakeNoise(256, 1);
            for (AmUInt64 i = 0; i < 256; ++i)
                source[3][i] = noise[0][i];

            for (const eChannelLayout output : { eChannelLayout_Surround_5_1, eChannelLayout_Surround_7_1 })
            {
                const AudioBuffer out = Render(source, output);
                const AmInt32 lfe = ChannelLayout::Get(output).GetLfeChannel();
                for (AmUInt16 c = 0; c < out.GetChannelCount(); ++c)
                    for (AmUInt64 i = 0; i < 256; ++i)
                        AM_EXPECT_EQ(c == lfe ? source[3][i] : 0.0f, out[c][i]);
            }

            // No LFE on the output: dropped, as in the ITU fold-down.
            const AudioBuffer onStereo = Render(source, eChannelLayout_Stereo);
            for (AmUInt16 c = 0; c < 2; ++c)
                for (AmUInt64 i = 0; i < 256; ++i)
                    AM_EXPECT_EQ(0.0f, onStereo[c][i]);
        }
    };

    AM_REGISTER_TEST(mixer_nodes, speaker_panning_node_lfe_bypasses_panning);
} // namespace SparkyStudios::Audio::Amplitude::Tests
