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
    AM_TEST_CASE(SpeakerPanningTestCase, mixer_nodes, speaker_panning_node_follows_spread_curve)
    {
    public:
        void Run() override
        {
            SetUpListener();
            GetMockLayer().SetSpatialization(eSpatialization_Position);
            GetMockLayer().SetLocation(LocationAt(30.0f));

            const AudioBuffer source = MakeNoise(256, 2);

            // Spread 0 set on the sound...
            MockSound narrow;
            narrow.SetSpread(0.0f);
            GetMockLayer().SetSound(&narrow);
            const AudioBuffer collapsed = Render(source, eChannelLayout_Stereo);
            GetMockLayer().SetSound(nullptr);

            // ...equals a native spread scaled to 0 by the attenuation's spread curve.
            MockAttenuation attenuation;
            attenuation.SetSpreadFactor(0.0f);
            GetMockLayer().SetAttenuation(&attenuation);
            AM_EXPECT(Near(Render(source, eChannelLayout_Stereo), collapsed, 1e-6f));

            // A factor of 1 leaves the native width.
            attenuation.SetSpreadFactor(1.0f);
            const AudioBuffer wide = Render(source, eChannelLayout_Stereo);
            GetMockLayer().SetAttenuation(nullptr);
            AM_EXPECT(Near(wide, Render(source, eChannelLayout_Stereo), 1e-6f));
        }
    };

    AM_REGISTER_TEST(mixer_nodes, speaker_panning_node_follows_spread_curve);
} // namespace SparkyStudios::Audio::Amplitude::Tests
