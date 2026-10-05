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

        AmUInt64 Pull(ResamplerInstance& resampler, const std::vector<AmReal32>& source, AmUInt64 frames, std::vector<AmReal32>& out)
        {
            AudioBuffer in(source.size(), 1);
            for (AmUInt64 i = 0; i < source.size(); ++i)
                in[0][i] = source[i];

            AudioBuffer o(frames, 1);
            AmUInt64 inFrames = resampler.GetInputFramesNeeded(frames);
            AmUInt64 outFrames = frames;
            if (!resampler.Process(in, inFrames, o, outFrames))
                return 0;

            for (AmUInt64 i = 0; i < outFrames; ++i)
                out.push_back(o[0][i]);
            return inFrames;
        }

        // The largest sample-to-sample jump, which is where a kink in the read position shows up as a click.
        AmReal32 LargestJump(const std::vector<AmReal32>& x)
        {
            AmReal32 worst = 0.0f;
            for (std::size_t i = 1; i < x.size(); ++i)
                worst = std::max(worst, std::abs(x[i] - x[i - 1]));
            return worst;
        }
    } // namespace

    AM_TEST_CASE(DSPTestCase, dsp_resamplers, resampler_ratio_ramp_joins_smoothly)
    {
    public:
        void Run() override
        {
            // Two ramps published back to back must join the way the ends of a ramp are built to join: the first
            // lands exactly on its end value with no slope left, the second leaves its start value with none. The
            // render is a constant-frequency tone, so a step in the read position is a jump in the output.
            constexpr AmReal64 kFirst = 0.5;
            constexpr AmReal64 kMiddle = 1.25;
            constexpr AmReal64 kLast = 2.0;
            constexpr AmUInt64 kFrames = 1024;

            const auto meanFrames = [](AmReal64 start, AmReal64 end)
            {
                return static_cast<AmReal64>(static_cast<AmReal64>(kFrames) * 0.5 * (start + end));
            };

            for (const char* name : kResamplerPresets)
            {
                const std::vector<AmReal32> source = Sine(kFrames * 8 + 8192, 3000.0, 48000.0);

                auto joined = Resampler::Construct(name);
                joined->Initialize(1, 48000, 48000);
                std::vector<AmReal32> across;
                AmUInt64 read = 0;

                joined->SetRatioRamp(kFirst, kMiddle, kFrames);
                read += Pull(*joined, source, kFrames, across);
                const AmUInt64 firstConsumed = read;

                joined->SetRatioRamp(kMiddle, kLast, kFrames);
                read += Pull(*joined, source, kFrames, across);
                const AmUInt64 secondConsumed = read - firstConsumed;

                AM_EXPECT_EQ(across.size(), static_cast<AmSize>(2 * kFrames));

                // Each ramp advances the stream by its own mean.
                AM_EXPECT(std::abs(static_cast<double>(firstConsumed) - static_cast<double>(meanFrames(kFirst, kMiddle))) <= 1.0);
                AM_EXPECT(std::abs(static_cast<double>(secondConsumed) - static_cast<double>(meanFrames(kMiddle, kLast))) <= 1.0);

                // And the read position reports the mean of what it was asked for, not the end it is heading to.
                const auto* instance = dynamic_cast<const BandlimitedResamplerInstance*>(joined.get());
                AM_EXPECT(instance != nullptr);
                if (instance != nullptr)
                    AM_EXPECT(std::abs(instance->GetRatio() - 0.5 * (kMiddle + kLast)) < 1e-12);

                // The junction is no rougher than either ramp's interior: a step in the read position would put the
                // largest jump of the whole render exactly there.
                const auto interior = std::max(
                    LargestJump(std::vector<AmReal32>(across.begin(), across.begin() + kFrames)),
                    LargestJump(std::vector<AmReal32>(across.begin() + kFrames, across.end())));
                const auto junction = static_cast<AmSize>(kFrames);
                AM_EXPECT(std::abs(across[junction] - across[junction - 1]) <= interior * 1.5f);
            }
        }
    };

    AM_REGISTER_TEST(dsp_resamplers, resampler_ratio_ramp_joins_smoothly);
} // namespace SparkyStudios::Audio::Amplitude::Tests