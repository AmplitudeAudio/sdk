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
    AM_TEST_CASE(SpeakerPanningTestCase, mixer_nodes, speaker_panning_node_spread_native_equals_2d)
    {
    public:
        void Run() override
        {
            SetUpListener();
            GetMockLayer().SetLocation(LocationAt(0.0f));

            const auto check = [&](AmUInt16 channels, eChannelLayout output)
            {
                const AudioBuffer source = MakeNoise(256, channels);

                GetMockLayer().SetSpatialization(eSpatialization_Position);
                const AudioBuffer spatial = Render(source, output);

                GetMockLayer().SetSpatialization(eSpatialization_None);
                const AudioBuffer flat = Render(source, output);

                AM_EXPECT(Near(spatial, flat, 1e-6f));
            };

            // No sound: the spread is the asset's native width. Straight ahead on a matching output, every channel lands
            // on its own speaker, as when the asset plays 2D.
            check(2, eChannelLayout_Stereo);
            check(4, eChannelLayout_Quad);
            check(6, eChannelLayout_Surround_5_1);
            check(8, eChannelLayout_Surround_7_1);

            // Stereo on 5.1 lands on L and R both ways.
            check(2, eChannelLayout_Surround_5_1);
        }
    };

    AM_REGISTER_TEST(mixer_nodes, speaker_panning_node_spread_native_equals_2d);
} // namespace SparkyStudios::Audio::Amplitude::Tests
