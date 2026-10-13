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

#include <Ambisonics/AmbisonicComponent.h>
#include <Mixer/Nodes/AmbisonicPanningNode.h>

#include "SpeakerPanningTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(EngineSpeakerPanningTestCase, mixer_nodes, ambisonic_panning_node_encodes_each_emitter)
    {
    public:
        void Run() override
        {
            constexpr AmUInt64 frames = 256;
            constexpr AmReal32 k = 0.70710678f;

            SetUpListener();
            GetMockLayer().SetSpatialization(eSpatialization_HRTF);
            GetMockLayer().SetLocation(LocationAt(20.0f, 5.0f));

            const auto encode = [&](const AudioBuffer& input)
            {
                AmbisonicPanningNode node;
                auto instance = node.CreateInstance();
                instance->Initialize(1, GetLayer(), nullptr, node.GetParameterCount());
                instance->Configure(frames, static_cast<AmUInt16>(input.GetChannelCount()));

                const AudioBuffer* field = AsProcessor(instance)->Process(&input);
                AudioBuffer copy(frames, instance->GetOutputChannelCount());
                AM_EXPECT_NOT(field == nullptr);
                if (field != nullptr)
                    AudioBuffer::Copy(*field, 0, copy, 0, frames);

                return copy;
            };

            const AudioBuffer noise = MakeNoise(frames, 1);
            AudioBuffer leftOnly(frames, 2);
            AudioBuffer rightOnly(frames, 2);
            AudioBuffer folded(frames, 1);
            for (AmUInt64 i = 0; i < frames; ++i)
            {
                leftOnly[0][i] = noise[0][i];
                rightOnly[1][i] = noise[0][i];
                folded[0][i] = k * noise[0][i];
            }

            MockSound sound;
            GetMockLayer().SetSound(&sound);

            // Spread 0: both emitters sit on the sound's direction, weighted by the fold-down. Same field as the
            // fold-down encoded as a mono source.
            sound.SetSpread(0.0f);
            AM_EXPECT(Near(encode(leftOnly), encode(folded), 1e-6f));

            // Native width: the two channels are encoded at mirrored positions. Same omni (W) part, different
            // directional parts.
            sound.SetSpread(kAmSpreadNative);
            const AudioBuffer left = encode(leftOnly);
            const AudioBuffer right = encode(rightOnly);

            AmReal64 directionalDifference = 0.0;
            for (AmUInt64 i = 0; i < frames; ++i)
            {
                AM_EXPECT(std::abs(left[eBFormatChannel_W][i] - right[eBFormatChannel_W][i]) < 1e-6f);
                directionalDifference += std::abs(left[eBFormatChannel_Y][i] - right[eBFormatChannel_Y][i]) +
                    std::abs(left[eBFormatChannel_X][i] - right[eBFormatChannel_X][i]);
            }
            AM_EXPECT(directionalDifference > 1.0);

            GetMockLayer().SetSound(nullptr);
        }
    };

    AM_REGISTER_TEST(mixer_nodes, ambisonic_panning_node_encodes_each_emitter);
} // namespace SparkyStudios::Audio::Amplitude::Tests
