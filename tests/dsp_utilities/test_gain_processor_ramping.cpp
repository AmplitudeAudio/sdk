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
#include <limits>
#include <vector>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <DSP/Gain.h>
#include <Utils/Utils.h>

#include "DSPTestCase.h"
#include "TestRegistry.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    namespace
    {
        constexpr AmReal32 kRampTolerance = 1e-4f;

        void FillChannel(AudioBufferChannel& channel, AmReal32 value)
        {
            for (AmSize i = 0; i < channel.size(); ++i)
                channel[i] = value;
        }
    } // namespace

    AM_TEST_CASE(DSPTestCase, dsp_utilities, gain_processor_first_call_snaps_to_target)
    {
    public:
        void Run() override
        {
            constexpr AmSize frames = 64;
            AudioBuffer input(frames, 1);
            AudioBuffer output(frames, 1);
            FillChannel(input[0], 1.0f);

            GainProcessor processor;
            processor.SetMinRampFrames(256);
            AM_EXPECT_NOT(processor.IsInitialized());

            processor.ApplyGain(0.5f, input[0], 0, output[0], 0, frames, false);

            for (AmSize i = 0; i < frames; ++i)
                AM_EXPECT(std::abs(output[0][i] - 0.5f) < kRampTolerance);

            AM_EXPECT(processor.IsInitialized());
            AM_EXPECT_NOT(processor.IsRamping());
            AM_EXPECT_EQ(processor.GetGain(), 0.5f);
        }
    };

    AM_REGISTER_TEST(dsp_utilities, gain_processor_first_call_snaps_to_target);

    AM_TEST_CASE(DSPTestCase, dsp_utilities, gain_processor_ramp_spans_blocks_linearly)
    {
    public:
        void Run() override
        {
            constexpr AmSize blockFrames = 64;
            constexpr AmSize rampFrames = 256;
            AudioBuffer input(blockFrames, 1);
            AudioBuffer output(blockFrames, 1);
            FillChannel(input[0], 1.0f);

            GainProcessor processor;
            processor.SetMinRampFrames(rampFrames);
            processor.Reset(0.0f);

            std::vector<AmReal32> rendered;
            for (AmSize block = 0; block < rampFrames / blockFrames; ++block)
            {
                processor.ApplyGain(1.0f, input[0], 0, output[0], 0, blockFrames, false);
                for (AmSize i = 0; i < blockFrames; ++i)
                    rendered.push_back(output[0][i]);

                if (block < rampFrames / blockFrames - 1)
                    AM_EXPECT(processor.IsRamping());
            }

            for (AmSize i = 0; i < rampFrames; ++i)
                AM_EXPECT(std::abs(rendered[i] - static_cast<AmReal32>(i) / static_cast<AmReal32>(rampFrames)) < kRampTolerance);

            AM_EXPECT_EQ(processor.GetGain(), 1.0f);
            AM_EXPECT_NOT(processor.IsRamping());
        }
    };

    AM_REGISTER_TEST(dsp_utilities, gain_processor_ramp_spans_blocks_linearly);

    AM_TEST_CASE(DSPTestCase, dsp_utilities, gain_processor_retarget_mid_ramp_is_continuous)
    {
    public:
        void Run() override
        {
            constexpr AmSize frames = 64;
            constexpr AmSize rampFrames = 256;
            AudioBuffer input(frames, 1);
            AudioBuffer first(frames, 1);
            AudioBuffer second(frames, 1);
            FillChannel(input[0], 1.0f);

            GainProcessor processor;
            processor.SetMinRampFrames(rampFrames);
            processor.Reset(0.0f);

            processor.ApplyGain(1.0f, input[0], 0, first[0], 0, frames, false);
            AM_EXPECT(std::abs(processor.GetGain() - 0.25f) < kRampTolerance);

            // Retarget downwards while the upward ramp is still running.
            processor.ApplyGain(0.0f, input[0], 0, second[0], 0, frames, false);

            const AmReal32 upStep = 1.0f / static_cast<AmReal32>(rampFrames);
            AM_EXPECT(std::abs(second[0][0] - first[0][frames - 1]) <= upStep + kRampTolerance);

            const AmReal32 downStep = 0.25f / static_cast<AmReal32>(rampFrames);
            for (AmSize i = 1; i < frames; ++i)
            {
                const AmReal32 delta = second[0][i - 1] - second[0][i];
                AM_EXPECT(delta >= 0.0f);
                AM_EXPECT(std::abs(delta - downStep) < kRampTolerance);
            }
        }
    };

    AM_REGISTER_TEST(dsp_utilities, gain_processor_retarget_mid_ramp_is_continuous);

    AM_TEST_CASE(DSPTestCase, dsp_utilities, gain_processor_ramps_over_whole_block)
    {
    public:
        void Run() override
        {
            constexpr AmSize frames = 512;
            AudioBuffer input(frames, 1);
            AudioBuffer output(frames, 1);
            FillChannel(input[0], 1.0f);

            GainProcessor processor;
            processor.SetMinRampFrames(256);
            processor.Reset(0.5f);

            // A small change must still ramp across the entire block (not a few samples).
            processor.ApplyGain(0.51f, input[0], 0, output[0], 0, frames, false);

            for (AmSize i = 0; i < frames; ++i)
            {
                const AmReal32 expected = 0.5f + 0.01f * static_cast<AmReal32>(i) / static_cast<AmReal32>(frames);
                AM_EXPECT(std::abs(output[0][i] - expected) < kRampTolerance);
            }

            AM_EXPECT_EQ(processor.GetGain(), 0.51f);
            AM_EXPECT_NOT(processor.IsRamping());
        }
    };

    AM_REGISTER_TEST(dsp_utilities, gain_processor_ramps_over_whole_block);

    AM_TEST_CASE(DSPTestCase, dsp_utilities, gain_processor_respects_offsets)
    {
    public:
        void Run() override
        {
            constexpr AmSize frames = 256;
            constexpr AmSize offset = 64;
            constexpr AmSize count = 128;
            AudioBuffer input(frames, 1);
            AudioBuffer output(frames, 1);
            FillChannel(input[0], 2.0f);

            auto expectUntouchedOutside = [&]()
            {
                for (AmSize i = 0; i < offset; ++i)
                    AM_EXPECT_EQ(output[0][i], 7.0f);
                for (AmSize i = offset + count; i < frames; ++i)
                    AM_EXPECT_EQ(output[0][i], 7.0f);
            };

            // Zero fast path.
            FillChannel(output[0], 7.0f);
            GainProcessor zero(0.0f);
            zero.ApplyGain(0.0f, input[0], offset, output[0], offset, count, false);
            for (AmSize i = offset; i < offset + count; ++i)
                AM_EXPECT_EQ(output[0][i], 0.0f);
            expectUntouchedOutside();

            // Unity fast path.
            FillChannel(output[0], 7.0f);
            GainProcessor unity(1.0f);
            unity.ApplyGain(1.0f, input[0], offset, output[0], offset, count, false);
            for (AmSize i = offset; i < offset + count; ++i)
                AM_EXPECT_EQ(output[0][i], 2.0f);
            expectUntouchedOutside();

            // Block-constant helpers must honor offsets too.
            FillChannel(output[0], 7.0f);
            Gain::ApplyReplaceConstantGain(0.0f, input[0], offset, output[0], offset, count);
            expectUntouchedOutside();

            FillChannel(output[0], 7.0f);
            Gain::ApplyReplaceConstantGain(1.0f, input[0], offset, output[0], offset, count);
            for (AmSize i = offset; i < offset + count; ++i)
                AM_EXPECT_EQ(output[0][i], 2.0f);
            expectUntouchedOutside();

            FillChannel(output[0], 7.0f);
            Gain::ApplyAccumulateConstantGain(1.0f, input[0], offset, output[0], offset, count);
            for (AmSize i = offset; i < offset + count; ++i)
                AM_EXPECT_EQ(output[0][i], 9.0f);
            expectUntouchedOutside();
        }
    };

    AM_REGISTER_TEST(dsp_utilities, gain_processor_respects_offsets);

    AM_TEST_CASE(DSPTestCase, dsp_utilities, gain_processor_reset_cancels_ramp)
    {
    public:
        void Run() override
        {
            constexpr AmSize frames = 64;
            AudioBuffer input(frames, 1);
            AudioBuffer output(frames, 1);
            FillChannel(input[0], 1.0f);

            GainProcessor processor;
            processor.SetMinRampFrames(256);
            processor.Reset(0.0f);
            processor.ApplyGain(1.0f, input[0], 0, output[0], 0, frames, false);
            AM_EXPECT(processor.IsRamping());

            processor.Reset(0.3f);
            AM_EXPECT_NOT(processor.IsRamping());
            AM_EXPECT_EQ(processor.GetGain(), 0.3f);

            processor.ApplyGain(0.3f, input[0], 0, output[0], 0, frames, false);
            for (AmSize i = 0; i < frames; ++i)
                AM_EXPECT(std::abs(output[0][i] - 0.3f) < kRampTolerance);

            processor.Invalidate();
            AM_EXPECT_NOT(processor.IsInitialized());
            processor.ApplyGain(0.8f, input[0], 0, output[0], 0, frames, false);
            AM_EXPECT_EQ(output[0][0], 0.8f);
        }
    };

    AM_REGISTER_TEST(dsp_utilities, gain_processor_reset_cancels_ramp);

    AM_TEST_CASE(DSPTestCase, dsp_utilities, gain_processor_non_finite_target_is_zero)
    {
    public:
        void Run() override
        {
            constexpr AmSize frames = 256;
            AudioBuffer input(frames, 1);
            AudioBuffer output(frames, 1);
            FillChannel(input[0], 1.0f);

            GainProcessor processor;
            processor.SetMinRampFrames(256);
            processor.Reset(1.0f);

            processor.ApplyGain(std::numeric_limits<AmReal32>::quiet_NaN(), input[0], 0, output[0], 0, frames, false);
            for (AmSize i = 0; i < frames; ++i)
                AM_EXPECT(std::isfinite(output[0][i]));
            AM_EXPECT_EQ(processor.GetGain(), 0.0f);

            processor.ApplyGain(std::numeric_limits<AmReal32>::infinity(), input[0], 0, output[0], 0, frames, false);
            AM_EXPECT_EQ(processor.GetGain(), 0.0f);

            processor.ApplyGain(0.5f, input[0], 0, output[0], 0, frames, false);
            AM_EXPECT_EQ(processor.GetGain(), 0.5f);
        }
    };

    AM_REGISTER_TEST(dsp_utilities, gain_processor_non_finite_target_is_zero);

    AM_TEST_CASE(DSPTestCase, dsp_utilities, gain_processor_handles_unaligned_blocks)
    {
    public:
        void Run() override
        {
            constexpr AmSize blockFrames = 100;
            constexpr AmSize offset = 3;
            constexpr AmSize blocks = 3;
            AudioBuffer input(blockFrames + offset, 1);
            AudioBuffer output(blockFrames + offset, 1);
            FillChannel(input[0], 1.0f);

            GainProcessor processor;
            processor.SetMinRampFrames(256);
            processor.Reset(0.0f);

            std::vector<AmReal32> rendered;
            for (AmSize block = 0; block < blocks; ++block)
            {
                processor.ApplyGain(1.0f, input[0], offset, output[0], offset, blockFrames, false);
                for (AmSize i = 0; i < blockFrames; ++i)
                    rendered.push_back(output[0][offset + i]);
            }

            // SIMD builds round the 256-frame ramp up to the SIMD block size.
            const AmSize rampFrames = AM_VALUE_ALIGN(AmSize(256), GetSimdBlockSize());
            for (AmSize i = 0; i < rendered.size(); ++i)
            {
                const AmReal32 expected = i < rampFrames ? static_cast<AmReal32>(i) / static_cast<AmReal32>(rampFrames) : 1.0f;
                AM_EXPECT(std::abs(rendered[i] - expected) < kRampTolerance);
            }

            AM_EXPECT_EQ(processor.GetGain(), 1.0f);
        }
    };

    AM_REGISTER_TEST(dsp_utilities, gain_processor_handles_unaligned_blocks);

    AM_TEST_CASE(DSPTestCase, dsp_utilities, gain_processor_advance_matches_apply)
    {
    public:
        void Run() override
        {
            constexpr AmSize frames = 64;
            AudioBuffer input(frames, 1);
            AudioBuffer output(frames, 1);
            FillChannel(input[0], 1.0f);

            GainProcessor applied;
            applied.SetMinRampFrames(256);
            applied.Reset(0.2f);

            GainProcessor advanced = applied;

            applied.ApplyGain(0.9f, input[0], 0, output[0], 0, frames, false);
            advanced.Advance(0.9f, frames);

            AM_EXPECT_EQ(applied.GetGain(), advanced.GetGain());
            AM_EXPECT_EQ(applied.IsRamping(), advanced.IsRamping());

            // Zero frames never alters state.
            const AmReal32 before = advanced.GetGain();
            advanced.Advance(0.1f, 0);
            advanced.ApplyGain(0.1f, input[0], 0, output[0], 0, 0, false);
            AM_EXPECT_EQ(advanced.GetGain(), before);
        }
    };

    AM_REGISTER_TEST(dsp_utilities, gain_processor_advance_matches_apply);
} // namespace SparkyStudios::Audio::Amplitude::Tests
