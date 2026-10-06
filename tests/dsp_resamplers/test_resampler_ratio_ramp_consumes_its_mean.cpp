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

        // Runs one ramp of @p frames output frames and reports what came out.
        struct Consumption
        {
            AmUInt64 consumed = 0;
            AmUInt64 produced = 0;
        };

        Consumption ConsumeRamp(ResamplerInstance& resampler, const std::vector<AmReal32>& source, AmUInt64 frames)
        {
            AudioBuffer in(source.size(), 1);
            for (AmUInt64 i = 0; i < source.size(); ++i)
                in[0][i] = source[i];

            AudioBuffer out(frames, 1);
            AmUInt64 inFrames = source.size();
            AmUInt64 outFrames = frames;
            if (!resampler.Process(in, inFrames, out, outFrames))
                return {};

            return { inFrames, outFrames };
        }
    } // namespace

    AM_TEST_CASE(DSPTestCase, dsp_resamplers, resampler_ratio_ramp_consumes_its_mean)
    {
    public:
        void Run() override
        {
            // A ramp that speeds the stream up, one that slows it down, and one that holds the mean in both
            // directions: all three must advance the stream by the mean, which is the whole basis of the
            // default SetRatioRamp() and of every caller that maps positions off GetRatio().
            constexpr AmReal64 kRamps[][3] = {
                { 0.5, 2.0, 1024.0 },
                { 2.0, 0.5, 1024.0 },
                { 0.75, 1.25, 997.0 },
                { 0.5, 2.0, 64.0 },
            };

            for (const char* name : kResamplerPresets)
            {
                for (const auto& ramp : kRamps)
                {
                    const auto start = ramp[0];
                    const auto end = ramp[1];
                    const auto frames = static_cast<AmUInt64>(ramp[2]);

                    const std::vector<AmReal32> source = Sine(frames * 4 + 8192, 997.0, 48000.0);

                    auto instance = Resampler::Construct(name);
                    instance->Initialize(1, 48000, 48000);
                    instance->SetRatioRamp(start, end, frames);

                    const Consumption run = ConsumeRamp(*instance, source, frames);
                    const auto expected = static_cast<AmUInt64>(static_cast<AmReal64>(frames) * 0.5 * (start + end));

                    AM_EXPECT_EQ(run.produced, frames);
                    AM_EXPECT(std::abs(static_cast<double>(run.consumed) - static_cast<double>(expected)) <= 1.0);
                }
            }
        }
    };

    AM_REGISTER_TEST(dsp_resamplers, resampler_ratio_ramp_consumes_its_mean);
} // namespace SparkyStudios::Audio::Amplitude::Tests
