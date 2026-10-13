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

#include <array>
#include <cmath>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Mixer/Nodes/AmbisonicPanningNode.h>
#include <Mixer/Nodes/AmbisonicSpeakerDecoderNode.h>

#include "SpeakerPanningTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(EngineSpeakerPanningTestCase, mixer_nodes, ambisonic_panning_node_places_left_channel_left)
    {
    public:
        void Run() override
        {
            constexpr AmUInt64 frames = 512;

            SetUpListener();
            GetMockLayer().SetSpatialization(eSpatialization_HRTF);
            // The ambisonic encoder reads world space (forward +Y, up +Z): straight ahead of the listener is +Y.
            GetMockLayer().SetLocation({ 0.0f, 5.0f, 0.0f });

            // Encodes the stereo input with the panner, then decodes the field to 5.1 and returns the energy per speaker.
            const auto render = [&](const AudioBuffer& input)
            {
                AmbisonicPanningNode panner;
                auto encoder = panner.CreateInstance();
                encoder->Initialize(1, GetLayer(), nullptr, panner.GetParameterCount());
                encoder->Configure(frames, 2);

                const AudioBuffer* field = AsProcessor(encoder)->Process(&input);
                AM_EXPECT_NOT(field == nullptr);

                std::array<AmReal64, kAmMaxLayoutChannelCount> energy{};
                if (field == nullptr)
                    return energy;

                AmbisonicSpeakerDecoderNode decoderNode;
                auto decoder = decoderNode.CreateInstance();
                decoder->Initialize(1, GetLayer(), nullptr, decoderNode.GetParameterCount());
                decoder->SetPipelineOutputLayout(eChannelLayout_Surround_5_1);
                decoder->Configure(frames, encoder->GetOutputChannelCount());

                const AudioBuffer* out = AsProcessor(decoder)->Process(field);
                AM_EXPECT_NOT(out == nullptr);
                if (out != nullptr)
                    for (AmUInt16 c = 0; c < out->GetChannelCount(); ++c)
                        for (AmUInt64 i = 0; i < frames; ++i)
                            energy[c] += static_cast<AmReal64>((*out)[c][i]) * (*out)[c][i];

                return energy;
            };

            const AudioBuffer noise = MakeNoise(frames, 1);
            AudioBuffer leftOnly(frames, 2);
            AudioBuffer rightOnly(frames, 2);
            for (AmUInt64 i = 0; i < frames; ++i)
            {
                leftOnly[0][i] = noise[0][i];
                rightOnly[1][i] = noise[0][i];
            }

            // Only the front pair is asserted: a stereo source sits at +-30 degrees, and the basic decode of such a source into the
            // +-110 degree surrounds has side lobes that make Rs as loud as Ls.
            const auto left = render(leftOnly);
            AM_EXPECT(left[0] > left[1]); // L over R

            const auto right = render(rightOnly);
            AM_EXPECT(right[1] > right[0]); // R over L
        }
    };

    AM_REGISTER_TEST(mixer_nodes, ambisonic_panning_node_places_left_channel_left);
} // namespace SparkyStudios::Audio::Amplitude::Tests
