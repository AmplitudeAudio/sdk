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
    AM_TEST_CASE(SpeakerPanningTestCase, mixer_nodes, speaker_panning_node_coincident_source_is_centred)
    {
    public:
        void Run() override
        {
            SetUpListener();
            GetMockLayer().SetSpatialization(eSpatialization_Position);
            GetMockLayer().SetLocation({ 0.0f, 0.0f, 0.0f });

            AudioBuffer source(256, 1);
            for (AmUInt64 i = 0; i < 256; ++i)
                source[0][i] = 1.0f;

            const AudioBuffer out = Render(source, eChannelLayout_Stereo);
            for (AmUInt64 i = 0; i < 256; ++i)
            {
                AM_EXPECT(std::abs(out[0][i] - 0.70710678f) < 1e-6f);
                AM_EXPECT(std::abs(out[1][i] - 0.70710678f) < 1e-6f);
            }

            const AudioBuffer onSurround = Render(source, eChannelLayout_Surround_5_1);
            AM_EXPECT(std::abs(onSurround[2][0] - 1.0f) < 1e-6f);
        }
    };

    AM_REGISTER_TEST(mixer_nodes, speaker_panning_node_coincident_source_is_centred);
} // namespace SparkyStudios::Audio::Amplitude::Tests
