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

#include <Ambisonics/AmbisonicShelfFilter.h>
#include <Ambisonics/BFormat.h>
#include <Ambisonics/SpeakerDecodingMatrix.h>
#include <Core/Engine.h>
#include <Core/EngineInternalState.h>
#include <Mixer/Nodes/AmbisonicSpeakerDecoderNode.h>
#include <Utils/Utils.h>

#include "SpeakerPanningTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(EngineSpeakerPanningTestCase, mixer_nodes, ambisonic_speaker_decoder_node_applies_shared_matrix)
    {
    public:
        void Run() override
        {
            constexpr AmUInt64 frames = 512;
            const AmUInt32 order = AM_MAX(static_cast<AmUInt32>(amEngine->GetPanningMode()), 1u);
            const AmUInt16 components = static_cast<AmUInt16>(OrderToComponents(order, true));

            const auto make = [&](eChannelLayout layout)
            {
                AmbisonicSpeakerDecoderNode node;
                auto instance = node.CreateInstance();
                instance->Initialize(1, GetLayer(), nullptr, node.GetParameterCount());
                instance->SetPipelineOutputLayout(layout);
                instance->Configure(frames, components);
                return instance;
            };

            // Two nodes on one layout share one matrix; another layout gets its own.
            auto a = make(eChannelLayout_Surround_5_1);
            auto b = make(eChannelLayout_Surround_5_1);
            auto q = make(eChannelLayout_Quad);
            const auto* na = dynamic_cast<AmbisonicSpeakerDecoderNodeInstance*>(a.get());
            const auto* nb = dynamic_cast<AmbisonicSpeakerDecoderNodeInstance*>(b.get());
            const auto* nq = dynamic_cast<AmbisonicSpeakerDecoderNodeInstance*>(q.get());
            AM_EXPECT(na != nullptr && nb != nullptr && nq != nullptr);
            AM_EXPECT(na->GetMatrix() != nullptr);
            AM_EXPECT(na->GetMatrix().get() == nb->GetMatrix().get());
            AM_EXPECT(nq->GetMatrix()->GetLayout() == eChannelLayout_Quad);
            AM_EXPECT(na->GetMatrix()->GetLayout() == eChannelLayout_Surround_5_1);

            // Output = shelf-filtered input times the matrix; the LFE stays silent.
            AudioBuffer input(frames, components);
            AmUInt32 seed = 5u;
            for (AmUInt16 c = 0; c < components; ++c)
                for (AmUInt64 i = 0; i < frames; ++i)
                {
                    seed = seed * 1664525u + 1013904223u;
                    input[c][i] = (static_cast<AmReal32>(seed >> 8) / static_cast<AmReal32>(1u << 24) - 0.5f) / static_cast<AmReal32>(c + 1);
                }

            const AudioBuffer* out = AsProcessor(a)->Process(&input);
            AM_EXPECT(out != nullptr);

            AmbisonicShelfFilter shelf;
            AM_EXPECT(shelf.Configure(order, true, static_cast<AmUInt32>(frames), GetLayer()->GetSampleRate()));
            shelf.SetHighFrequencyGains(na->GetMatrix()->GetHighFrequencyGains());
            BFormat field;
            field.Configure(order, true, static_cast<AmUInt32>(frames));
            for (AmUInt16 c = 0; c < components; ++c)
                field.CopyStream(input[c], c, static_cast<AmUInt32>(frames));
            shelf.Process(&field, static_cast<AmUInt32>(frames));

            const ChannelLayout& layout = ChannelLayout::Get(eChannelLayout_Surround_5_1);
            AmReal32 maxDiff = 0.0f;
            for (AmUInt16 s = 0; s < layout.GetChannelCount(); ++s)
            {
                for (AmUInt64 i = 0; i < frames; ++i)
                {
                    AmReal32 expected = 0.0f;
                    for (AmUInt16 c = 0; c < components; ++c)
                        expected += na->GetMatrix()->GetCoefficient(s, c) * field.GetBufferChannel(c)[i];
                    maxDiff = std::max(maxDiff, std::abs((*out)[s][i] - expected));
                    if (layout.IsLfe(s))
                        AM_EXPECT_EQ(0.0f, (*out)[s][i]);
                }
            }
            AM_EXPECT(maxDiff <= 1e-5f);

            // The matrix outlives the engine's cache.
            std::weak_ptr<const SpeakerDecodingMatrix> weak = na->GetMatrix();
            amEngine->GetState()->speaker_decoders.clear();
            AM_EXPECT(weak.lock() != nullptr);

            // Stereo renders (folded front image) instead of refusing.
            auto s = make(eChannelLayout_Stereo);
            AM_EXPECT(dynamic_cast<AmbisonicSpeakerDecoderNodeInstance*>(s.get())->GetMatrix() != nullptr);

            // No sample rate yet: the shelf filter refuses, the node stays silent instead of crashing.
            GetMockLayer().SetSampleRate(0);
            auto silent = make(eChannelLayout_Surround_5_1);
            const AudioBuffer* quiet = AsProcessor(silent)->Process(&input);
            AM_EXPECT(quiet != nullptr);
            bool allZero = true;
            for (AmUInt16 c = 0; quiet != nullptr && c < quiet->GetChannelCount(); ++c)
                for (AmUInt64 i = 0; i < frames; ++i)
                    allZero = allZero && (*quiet)[c][i] == 0.0f;
            AM_EXPECT(allZero);
            GetMockLayer().SetSampleRate(48000);
        }
    };

    AM_REGISTER_TEST(mixer_nodes, ambisonic_speaker_decoder_node_applies_shared_matrix);
} // namespace SparkyStudios::Audio::Amplitude::Tests
