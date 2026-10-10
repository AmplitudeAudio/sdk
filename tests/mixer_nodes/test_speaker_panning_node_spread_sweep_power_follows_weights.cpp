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
    AM_TEST_CASE(SpeakerPanningTestCase, mixer_nodes, speaker_panning_node_spread_sweep_power_follows_weights)
    {
    public:
        void Run() override
        {
            SetUpListener();
            GetMockLayer().SetSpatialization(eSpatialization_Position);
            GetMockLayer().SetLocation(LocationAt(0.0f));

            MockSound sound;
            GetMockLayer().SetSound(&sound);

            const ChannelLayout& stereo = ChannelLayout::Get(eChannelLayout_Stereo);
            const AudioBuffer source = MakeNoise(48000, 2);
            const AmReal64 channelPower[2] = { MeanSquare(source, 0), MeanSquare(source, 1) };

            AmReal64 atZero = 0.0;
            AmReal64 atNative = 0.0;

            for (const AmReal32 spread : { 0.0f, 15.0f, 30.0f, 60.0f, 120.0f, 240.0f, 360.0f })
            {
                sound.SetSpread(spread);
                const AudioBuffer out = Render(source, eChannelLayout_Surround_5_1);

                AmReal64 power = 0.0;
                for (AmUInt16 c = 0; c < 6; ++c)
                    power += MeanSquare(out, c);

                // Uncorrelated channels: the output power is the weighted sum of the channel powers.
                const SpreadEmitterSet emitters = SpreadEmitters::Compute(stereo, 0.0f, 0.0f, spread);
                AmReal64 expected = 0.0;
                for (AmUInt16 e = 0; e < emitters.count; ++e)
                    expected += static_cast<AmReal64>(emitters.emitters[e].weight) * emitters.emitters[e].weight *
                        channelPower[emitters.emitters[e].channel];

                AM_EXPECT(std::abs(10.0 * std::log10(power / expected)) < 0.2);

                if (spread == 0.0f)
                    atZero = power;
                if (spread == 60.0f)
                    atNative = power;
            }

            // A stereo asset rises 3.01 dB from spread 0 to its native width.
            AM_EXPECT(std::abs(10.0 * std::log10(atNative / atZero) - 3.0103) < 0.2);

            GetMockLayer().SetSound(nullptr);
        }
    };

    AM_REGISTER_TEST(mixer_nodes, speaker_panning_node_spread_sweep_power_follows_weights);
} // namespace SparkyStudios::Audio::Amplitude::Tests
