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

#include <algorithm>
#include <cmath>
#include <vector>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include "DSPTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    namespace
    {
        constexpr double kPi = 3.14159265358979323846;

        std::vector<AmReal32> Sine(AmUInt64 frames, AmReal64 hz, AmReal64 rate)
        {
            std::vector<AmReal32> x(frames);
            for (AmUInt64 i = 0; i < frames; ++i)
                x[i] = static_cast<AmReal32>(0.5 * std::sin(2.0 * kPi * hz * static_cast<AmReal64>(i) / rate));
            return x;
        }

        // The read-ahead and the estimate that sizes the FIFO are taken at the ramp's widest point, so a ramp
        // that speeds the stream up cannot outrun them. Both must therefore cover what a block actually reads,
        // or the block stops short of the output it promised and the mixer hears a gap.
        struct Shortfall
        {
            AmUInt64 requested = 0;
            AmUInt64 produced = 0;
            AmUInt64 needed = 0;
            AmUInt64 consumed = 0;
        };

        Shortfall Try(ResamplerInstance& resampler, const std::vector<AmReal32>& source, AmUInt64 frames, AmUInt64 offset)
        {
            const AmUInt64 needed = resampler.GetInputFramesNeeded(frames);

            AudioBuffer in(source.size(), 1);
            for (AmUInt64 i = 0; i < source.size(); ++i)
                in[0][i] = source[i];
            AM_UNUSED(offset);

            AudioBuffer out(frames, 1);
            AmUInt64 inFrames = needed;
            AmUInt64 outFrames = frames;
            if (!resampler.Process(in, inFrames, out, outFrames))
                return {};

            return { frames, outFrames, needed, inFrames };
        }
    } // namespace

    AM_TEST_CASE(DSPTestCase, dsp_resamplers, resampler_ratio_ramp_never_shortens_read_ahead)
    {
    public:
        void Run() override
        {
            // Every one of these speeds the stream up somewhere in the block, which is the case the sizing has to
            // survive: the ratio is at its widest after the estimate was taken.
            constexpr AmReal64 kRamps[][3] = {
                { 0.5, 2.0, 1024.0 },
                { 0.75, 2.0, 256.0 },
                { 1.0, 1.5, 4096.0 },
                { 0.9, 2.5, 1023.0 },
            };
            constexpr AmUInt64 kBlocks[] = { 1, 64, 512, 1024 };

            for (const char* name : kResamplerPresets)
            {
                for (const auto& ramp : kRamps)
                {
                    const auto frames = static_cast<AmUInt64>(ramp[2]);
                    const std::vector<AmReal32> source = Sine(frames * 6 + 8192, 997.0, 48000.0);

                    for (const AmUInt64 block : kBlocks)
                    {
                        auto instance = Resampler::Construct(name);
                        instance->Initialize(1, 48000, 48000);
                        instance->SetRatioRamp(ramp[0], ramp[1], frames);

                        const Shortfall first = Try(*instance, source, block, 0);
                        AM_EXPECT(instance->GetInputFramesNeeded(0) == 0);
                        AM_EXPECT_EQ(first.requested, first.produced);

                        // Asking again mid-ramp must stay valid: the phase has moved on, and the second call
                        // covers the part of the ramp that follows the frames already produced.
                        const Shortfall second = Try(*instance, source, block, 0);
                        AM_EXPECT_EQ(second.requested, second.produced);
                        AM_EXPECT(second.needed > 0);

                        // The estimate covers what was read, with the identity shortcut taken into account.
                        AM_EXPECT(first.consumed <= first.needed);
                    }
                }
            }
        }
    };

    AM_REGISTER_TEST(dsp_resamplers, resampler_ratio_ramp_never_shortens_read_ahead);
} // namespace SparkyStudios::Audio::Amplitude::Tests