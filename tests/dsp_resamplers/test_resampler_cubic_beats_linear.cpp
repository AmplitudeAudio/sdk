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

        /**
         * @brief The RMS left over after fitting a pure tone of @p hz to a stretch of @p out.
         *
         * Gain and phase are fitted rather than assumed, so the figure is interpolation error and harmonic
         * distortion alone -- which is exactly what separates one interpolation kernel from another. The pitch is
         * preserved by the resampler, so the analytic sine is a pure tone at @p hz sampled at @p rate.
         */
        double ResidualAgainstSine(const std::vector<AmReal32>& out, AmUInt64 begin, AmUInt64 end, AmReal64 hz, AmReal64 rate)
        {
            double cc = 0.0;
            double ss = 0.0;
            double cs = 0.0;
            double yc = 0.0;
            double ys = 0.0;
            for (AmUInt64 i = begin; i < end; ++i)
            {
                const auto angle = 2.0 * kPi * hz * static_cast<AmReal64>(i) / rate;
                const double c = std::cos(angle);
                const double s = std::sin(angle);
                cc += c * c;
                ss += s * s;
                cs += c * s;
                yc += static_cast<double>(out[i]) * c;
                ys += static_cast<double>(out[i]) * s;
            }

            const double det = cc * ss - cs * cs;
            if (std::abs(det) < 1e-9)
                return std::numeric_limits<double>::infinity();

            const double a = (yc * ss - ys * cs) / det;
            const double b = (cc * ys - cs * yc) / det;

            double sum = 0.0;
            for (AmUInt64 i = begin; i < end; ++i)
            {
                const auto angle = 2.0 * kPi * hz * static_cast<AmReal64>(i) / rate;
                const double residual = static_cast<double>(out[i]) - (a * std::cos(angle) + b * std::sin(angle));
                sum += residual * residual;
            }

            return std::sqrt(sum / static_cast<double>(end - begin));
        }

        std::vector<AmReal32> Render(
            ResamplerInstance& resampler, const std::vector<AmReal32>& source, AmUInt64 blockSize, AmUInt64 totalOut)
        {
            std::vector<AmReal32> out;
            AmUInt64 read = 0;

            AudioBuffer in(source.size(), 1);
            AudioBuffer o(blockSize, 1);

            while (out.size() < totalOut)
            {
                const AmUInt64 want = std::min<AmUInt64>(blockSize, totalOut - out.size());
                const AmUInt64 needed = resampler.GetInputFramesNeeded(want);
                if (read + needed > source.size())
                    break;

                for (AmUInt64 i = 0; i < needed; ++i)
                    in[0][i] = source[read + i];

                AmUInt64 inFrames = needed;
                AmUInt64 outFrames = want;
                if (!resampler.Process(in, inFrames, o, outFrames) || outFrames != want)
                    break;

                read += inFrames;
                for (AmUInt64 i = 0; i < outFrames; ++i)
                    out.push_back(o[0][i]);
            }

            return out;
        }
    } // namespace

    // The linear and cubic presets are the only two hand-written polynomials in the resampler, and nothing else in
    // the suite distinguishes them: routing Cubic at the linear branch would leave every other test green. A tone at a
    // non-unity ratio is where the two diverge most -- linear interpolation is second-order accurate and the
    // Catmull-Rom cubic is fourth-order -- so the residual against the analytic sine separates them cleanly.
    AM_TEST_CASE(DSPTestCase, dsp_resamplers, resampler_cubic_beats_linear)
    {
    public:
        void Run() override
        {
            constexpr AmUInt32 sampleRateIn = 48000;
            constexpr AmUInt32 sampleRateOut = 44100;
            constexpr AmReal64 hz = 5000.0;
            constexpr AmUInt64 totalOut = 8192;

            std::vector<AmReal32> source(65536);
            for (AmSize i = 0; i < source.size(); ++i)
                source[i] = static_cast<AmReal32>(0.5 * std::sin(2.0 * kPi * hz * static_cast<AmReal64>(i) / sampleRateIn));

            std::vector<AmReal32> renders[2];
            for (const char* name : { "linear", "cubic" })
            {
                auto resampler = Resampler::Construct(name);
                resampler->Initialize(1, sampleRateIn, sampleRateOut);
                renders[std::string{ name } == "cubic" ? 1 : 0] = Render(*resampler, source, 1024, totalOut);
            }

            const std::vector<AmReal32>& linear = renders[0];
            const std::vector<AmReal32>& cubic = renders[1];

            AM_EXPECT_EQ(static_cast<AmSize>(totalOut), linear.size());
            AM_EXPECT_EQ(static_cast<AmSize>(totalOut), cubic.size());
            if (linear.size() != totalOut || cubic.size() != totalOut)
                return;

            // A silence would make both residuals zero and the comparison below meaningless.
            for (const std::vector<AmReal32>* render : { &linear, &cubic })
            {
                AmReal32 peak = 0.0f;
                for (const AmReal32 sample : *render)
                    peak = std::max(peak, std::abs(sample));
                AM_EXPECT(peak > 0.25f);
            }

            // Skip the kernel warm-up: the first outputs are filtered against the zeroed history.
            constexpr AmUInt64 begin = 1024;
            const double linearResidual = ResidualAgainstSine(linear, begin, totalOut, hz, sampleRateOut);
            const double cubicResidual = ResidualAgainstSine(cubic, begin, totalOut, hz, sampleRateOut);

            AM_EXPECT(linearResidual > 0.0);
            AM_EXPECT(cubicResidual > 0.0);
            AM_EXPECT(cubicResidual < linearResidual);

            // And the two presets do not render the same.
            AmReal32 worstDifference = 0.0f;
            for (AmSize i = begin; i < totalOut; ++i)
                worstDifference = std::max(worstDifference, std::abs(cubic[i] - linear[i]));
            AM_EXPECT(worstDifference > 1e-3f);
        }
    };

    AM_REGISTER_TEST(dsp_resamplers, resampler_cubic_beats_linear);
} // namespace SparkyStudios::Audio::Amplitude::Tests