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

        // The frequency of a tone over a window, from the interpolated positions of its rising zero crossings.
        // Interpolating is what makes a short window usable: a plain count is quantised to one cycle per window,
        // which is coarser than the frequency steps this test is trying to measure.
        AmReal64 Frequency(const std::vector<AmReal32>& x, AmUInt64 begin, AmUInt64 end, AmReal64 rate)
        {
            AmReal64 first = 0.0;
            AmReal64 last = 0.0;
            AmReal64 crossings = 0.0;

            for (AmUInt64 i = begin + 1; i < end; ++i)
            {
                const auto a = static_cast<AmReal64>(x[i - 1]);
                const auto b = static_cast<AmReal64>(x[i]);
                if (!(a <= 0.0 && b > 0.0))
                    continue;

                const auto at = static_cast<AmReal64>(i) - a / (b - a);
                if (crossings == 0.0)
                    first = at;
                last = at;
                crossings += 1.0;
            }

            if (crossings < 2.0 || last <= first)
                return 0.0;

            return rate * (crossings - 1.0) / (last - first);
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
            //
            // The two ramps deliberately span different amounts, because equal spans join smoothly under any
            // interpolation: a linear ramp across 0.75 and a linear ramp across 0.75 meet with the same slope, so
            // that configuration cannot tell a C1 join from a C0 one. Differing spans are what make the junction
            // a genuine test of whether the slope survives it.
            constexpr AmReal64 kFirst = 0.5;
            constexpr AmReal64 kMiddle = 1.25;
            constexpr AmReal64 kLast = 3.0;
            constexpr AmUInt64 kFrames = 1024;
            constexpr AmUInt64 kProbe = 128;
            constexpr AmReal64 kRate = 48000.0;
            constexpr auto kProbes = static_cast<AmSize>(kFrames / kProbe);

            const auto meanFrames = [](AmReal64 start, AmReal64 end)
            {
                return static_cast<AmReal64>(static_cast<AmReal64>(kFrames) * 0.5 * (start + end));
            };

            for (const char* name : kResamplerPresets)
            {
                const std::vector<AmReal32> source = Sine(kFrames * 8 + 8192, 3000.0, kRate);

                auto joined = Resampler::Construct(name);
                joined->Initialize(1, 48000, 48000);
                std::vector<AmReal32> across;
                AmUInt64 read = 0;

                // Pulled in probes, so a ramp also spans several Process() calls.
                joined->SetRatioRamp(kFirst, kMiddle, kFrames);
                for (AmUInt64 i = 0; i < kFrames; i += kProbe)
                    read += Pull(*joined, source, kProbe, across);
                const AmUInt64 firstConsumed = read;

                joined->SetRatioRamp(kMiddle, kLast, kFrames);
                for (AmUInt64 i = 0; i < kFrames; i += kProbe)
                    read += Pull(*joined, source, kProbe, across);
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

                // What a C1 join means is that the read position's *derivative* -- the step, and so the rendered
                // frequency -- is continuous across the boundary. Measuring the biggest sample-to-sample jump
                // cannot see that: the read position is continuous either way, so a kink in its slope barely moves
                // any individual sample, and the carrier dominates the measurement. Measure the slope directly
                // instead, as the frequency each window renders, and compare the step across the junction with the
                // steps inside the ramps.
                std::vector<AmReal64> frequency;
                frequency.reserve(2 * kProbes);
                for (AmSize i = 0; i < 2 * kProbes; ++i)
                    frequency.push_back(Frequency(across, i * kProbe, (i + 1) * kProbe, kRate));

                AM_EXPECT(*std::min_element(frequency.begin(), frequency.end()) > 0.0);

                std::vector<AmReal64> change;
                for (std::size_t i = 1; i < frequency.size(); ++i)
                    change.push_back(std::abs(frequency[i] - frequency[i - 1]) / frequency[i - 1]);

                // kProbes - 1 is the step from the last window of the first ramp into the first window of the
                // second; every other step lies inside a ramp.
                const AmReal64 junction = change[kProbes - 1];
                AmReal64 interior = 0.0;
                for (std::size_t i = 0; i < change.size(); ++i)
                {
                    if (i == kProbes - 1)
                        continue;
                    interior = std::max(interior, change[i]);
                }

                // A break in the slope at the junction would show up as the largest step in the render. With the
                // slope eased to zero on both sides it is instead among the smallest, so the boundary is bounded
                // well below the ramps' interiors rather than merely at par with them: the eased implementation
                // measures about 0.16 of the interior, a linear ramp across the junction about 0.63.
                AM_EXPECT(junction <= interior);
                AM_EXPECT(junction <= interior * 0.3);
            }
        }
    };

    AM_REGISTER_TEST(dsp_resamplers, resampler_ratio_ramp_joins_smoothly);
} // namespace SparkyStudios::Audio::Amplitude::Tests
