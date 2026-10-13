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

#include <Ambisonics/AmbisonicSource.h>
#include <Ambisonics/BFormat.h>
#include <Mixer/Nodes/AmbisonicSpeakerDecoderNode.h>
#include <Utils/Utils.h>

#include "SpeakerPanningTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(EngineSpeakerPanningTestCase, mixer_nodes, ambisonic_speaker_decoder_node_places_sources)
    {
    public:
        void Run() override
        {
            constexpr AmUInt64 frames = 512;

            const ePanningMode mode = amEngine->GetPanningMode();
            const AmUInt32 order = AM_MAX(static_cast<AmUInt32>(mode), 1u);
            const AmUInt16 components = static_cast<AmUInt16>(OrderToComponents(order, true));

            AudioBuffer tone(frames, 1);
            for (AmUInt64 i = 0; i < frames; ++i)
                tone[0][i] = std::sin(0.05f * static_cast<AmReal32>(i));

            // Encodes the tone the way AmbisonicPanning does: encoder azimuth, negative to the left.
            const auto encode = [&](AmReal32 encoderAzimuthDegrees)
            {
                AmbisonicSource source;
                source.Configure(order, true);
                source.SetPosition(SphericalPosition::FromDegrees(encoderAzimuthDegrees, 0.0f, 1.0f));

                BFormat field;
                field.Configure(order, true, static_cast<AmUInt32>(frames));
                field.Reset();
                source.Process(tone[0], static_cast<AmUInt32>(frames), &field);

                AudioBuffer copy(frames, components);
                AudioBuffer::Copy(*field.GetBuffer(), 0, copy, 0, frames);
                return copy;
            };

            const auto decode = [&](const AudioBuffer& field, eChannelLayout layout)
            {
                AmbisonicSpeakerDecoderNode node;
                auto instance = node.CreateInstance();
                instance->Initialize(1, GetLayer(), nullptr, node.GetParameterCount());
                instance->SetPipelineOutputLayout(layout);
                instance->Configure(frames, components);
                AM_EXPECT_EQ(ChannelLayout::Get(layout).GetChannelCount(), instance->GetOutputChannelCount());

                const AudioBuffer* out = AsProcessor(instance)->Process(&field);
                std::array<AmReal64, kAmMaxLayoutChannelCount> energy{};
                AM_EXPECT_NOT(out == nullptr);
                if (out != nullptr)
                    for (AmUInt16 c = 0; c < out->GetChannelCount(); ++c)
                        for (AmUInt64 i = 0; i < frames; ++i)
                            energy[c] += static_cast<AmReal64>((*out)[c][i]) * (*out)[c][i];

                return energy;
            };

            const AudioBuffer left = encode(-90.0f);
            const AudioBuffer front = encode(0.0f);

            const auto left51 = decode(left, eChannelLayout_Surround_5_1);
            AM_EXPECT(left51[0] > left51[1]); // L over R
            AM_EXPECT(left51[4] > left51[5]); // Ls over Rs
            AM_EXPECT_EQ(0.0, left51[3]);     // LFE silent

            const auto front51 = decode(front, eChannelLayout_Surround_5_1);
            AM_EXPECT(front51[2] > front51[0] && front51[2] > front51[4]); // C loudest
            AM_EXPECT(std::abs(front51[0] - front51[1]) <= 1e-6 * (front51[0] + 1.0)); // L = R

            const auto left71 = decode(left, eChannelLayout_Surround_7_1);
            AM_EXPECT(left71[6] > left71[7]); // side left over side right
            AM_EXPECT_EQ(0.0, left71[3]);

            const auto leftQuad = decode(left, eChannelLayout_Quad);
            AM_EXPECT(leftQuad[0] > leftQuad[1] && leftQuad[2] > leftQuad[3]);

            const auto leftStereo = decode(left, eChannelLayout_Stereo);
            AM_EXPECT(leftStereo[0] > leftStereo[1]);
        }
    };

    AM_REGISTER_TEST(mixer_nodes, ambisonic_speaker_decoder_node_places_sources);
} // namespace SparkyStudios::Audio::Amplitude::Tests
