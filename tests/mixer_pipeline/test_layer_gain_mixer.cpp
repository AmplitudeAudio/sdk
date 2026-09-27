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

#include <Mixer/LayerGainMixer.h>

#include "DSPTestCase.h"
#include "TestRegistry.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    namespace
    {
        constexpr AmSize kFrames = 256;
        constexpr AmReal32 kTolerance = 1e-4f;

        void Fill(AudioBuffer& buffer, AmReal32 value)
        {
            for (AmUInt16 c = 0; c < buffer.GetChannelCount(); ++c)
                for (AmSize i = 0; i < buffer.GetFrameCount(); ++i)
                    buffer[c][i] = value;
        }

        void Configure(GainProcessor (&processors)[kAmplimixMaxOutputChannels], AmReal32 gain)
        {
            for (auto& processor : processors)
            {
                processor.SetMinRampFrames(256);
                processor.Reset(gain);
            }
        }
    } // namespace

    AM_TEST_CASE(DSPTestCase, mixer_pipeline, layer_gain_mixer_ramps_across_blocks)
    {
    public:
        void Run() override
        {
            AudioBuffer in(kFrames, 2);
            AudioBuffer out(kFrames, 2);
            Fill(in, 1.0f);

            GainProcessor processors[kAmplimixMaxOutputChannels];
            Configure(processors, 0.25f);

            Fill(out, 0.0f);
            MixLayerWithGain(processors, 2, 0.25f, in, out, kFrames);
            const AmReal32 lastFirstBlock = out[0][kFrames - 1];
            AM_EXPECT(std::abs(lastFirstBlock - 0.25f) < kTolerance);

            // Accumulates onto existing content, ramping 0.25 -> 1 across the block.
            Fill(out, 0.1f);
            MixLayerWithGain(processors, 2, 1.0f, in, out, kFrames);

            const AmReal32 step = 0.75f / static_cast<AmReal32>(kFrames);
            for (AmUInt16 c = 0; c < 2; ++c)
            {
                AM_EXPECT(std::abs(out[c][0] - 0.35f) < kTolerance);
                for (AmSize i = 1; i < kFrames; ++i)
                    AM_EXPECT(std::abs((out[c][i] - out[c][i - 1]) - step) < kTolerance);
            }

            AM_EXPECT_EQ(processors[0].GetGain(), 1.0f);
            AM_EXPECT_EQ(processors[1].GetGain(), 1.0f);
        }
    };

    AM_REGISTER_TEST(mixer_pipeline, layer_gain_mixer_ramps_across_blocks);

    AM_TEST_CASE(DSPTestCase, mixer_pipeline, layer_gain_mixer_instances_share_one_ramp)
    {
    public:
        void Run() override
        {
            constexpr AmSize frames = 128;
            constexpr AmUInt32 instances = 3;

            AudioBuffer in(frames, 2);
            AudioBuffer out(frames, 2);
            Fill(in, 1.0f);

            GainProcessor processors[kAmplimixMaxOutputChannels];
            Configure(processors, 0.0f);

            // Block 1: every instance sees the same 0 -> 1 ramp (first half of 256 frames).
            Fill(out, 0.0f);
            for (AmUInt32 n = 0; n < instances; ++n)
                MixLayerInstanceWithGain(processors, 2, 1.0f, in, out, frames);

            for (AmSize i = 0; i < frames; ++i)
                AM_EXPECT(std::abs(out[0][i] - instances * static_cast<AmReal32>(i) / 256.0f) < kTolerance);

            // Instance mixing never advances the layer's processors...
            AM_EXPECT_EQ(processors[0].GetGain(), 0.0f);

            // ...the layer advances exactly once per block.
            AdvanceLayerGain(processors, 2, 1.0f, frames);
            AM_EXPECT(std::abs(processors[0].GetGain() - 0.5f) < kTolerance);
            AM_EXPECT(processors[0].IsRamping());

            // Block 2 continues the same ramp (second half).
            Fill(out, 0.0f);
            for (AmUInt32 n = 0; n < instances; ++n)
                MixLayerInstanceWithGain(processors, 2, 1.0f, in, out, frames);

            for (AmSize i = 0; i < frames; ++i)
                AM_EXPECT(std::abs(out[1][i] - instances * (0.5f + static_cast<AmReal32>(i) / 256.0f)) < kTolerance);
        }
    };

    AM_REGISTER_TEST(mixer_pipeline, layer_gain_mixer_instances_share_one_ramp);

    AM_TEST_CASE(DSPTestCase, mixer_pipeline, layer_gain_mixer_mono_touches_one_channel)
    {
    public:
        void Run() override
        {
            AudioBuffer in(kFrames, 2);
            AudioBuffer out(kFrames, 2);
            Fill(in, 1.0f);
            Fill(out, 0.0f);

            GainProcessor processors[kAmplimixMaxOutputChannels];
            Configure(processors, 0.5f);

            MixLayerWithGain(processors, 1, 0.5f, in, out, kFrames);

            for (AmSize i = 0; i < kFrames; ++i)
            {
                AM_EXPECT(std::abs(out[0][i] - 0.5f) < kTolerance);
                AM_EXPECT_EQ(out[1][i], 0.0f);
            }
        }
    };

    AM_REGISTER_TEST(mixer_pipeline, layer_gain_mixer_mono_touches_one_channel);
} // namespace SparkyStudios::Audio::Amplitude::Tests
