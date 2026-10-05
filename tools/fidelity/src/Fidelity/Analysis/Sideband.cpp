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

#include <Fidelity/Analysis/Sideband.h>

#include <algorithm>
#include <cmath>
#include <complex>
#include <vector>

#include <Fidelity/Analysis/Fft.h>
#include <Fidelity/Analysis/Window.h>

namespace SparkyStudios::Audio::Amplitude::Fidelity
{
    namespace
    {
        constexpr double kTwoPi = 6.28318530717958647692;

        /**
         * @brief The unwrapped phase of the expected pitch over the analysis window.
         *
         * @param[in] offsetSamples  The render is slid this many samples against the reference. A slide in time is a
         *                           phase of frequency times the slide, not a phase of the slide itself: adding the
         *                           slide raw would hand the demodulator a ramp the render never had.
         */
        std::vector<double> ReferencePhase(
            const Signal& trackedHz, std::size_t begin, std::size_t frames, double sampleRate, double offsetSamples)
        {
            std::vector<double> phase(frames);

            double cycles = 0.0;
            for (std::size_t n = 0; n < frames; ++n)
            {
                phase[n] = kTwoPi * (cycles + trackedHz[begin + n] * offsetSamples / sampleRate);
                cycles += trackedHz[begin + n] / sampleRate;
            }

            return phase;
        }
    } // namespace

    GlideSidebandResult AnalyzeGlideSidebands(
        std::span<const double> x, const Signal& trackedHz, const GlideSidebandOptions& options)
    {
        GlideSidebandResult result;
        if (options.sampleRate <= 0.0 || options.blockRateHz <= 0.0 || options.orders == 0 || options.innerBlockRates <= 0.0)
            return result;

        const std::size_t regionEnd = options.end > options.begin ? options.end : x.size();
        if (options.begin >= regionEnd || options.begin >= trackedHz.size())
            return result;

        const std::size_t available = std::min(regionEnd - options.begin, trackedHz.size() - options.begin);
        const std::size_t frames = std::min(options.frames, available);
        if (frames < 4)
            return result;

        const std::vector<double> window = BlackmanHarris7(frames);
        const double binHz = options.sampleRate / static_cast<double>(frames);
        const auto binOf = [&](double hz)
        {
            return static_cast<std::size_t>(std::llround(hz / binHz));
        };

        // The slide that leaves the most carrier behind: the render is not asked to match the command to the sample,
        // because the engine eases its speed and lags the command by a block or two, and over a glide that lag
        // walks a demodulator that was handed the raw command across several cycles of phase.
        const auto span = static_cast<double>(std::llround(options.fitSpanSeconds * options.sampleRate));
        double bestOffset = 0.0;
        double bestCarrier = 0.0;

        for (std::size_t step = 0; step <= options.fitSteps; ++step)
        {
            const double offset =
                2.0 * static_cast<double>(step) * span / static_cast<double>(options.fitSteps) - span;
            const std::vector<double> phase = ReferencePhase(trackedHz, options.begin, frames, options.sampleRate, offset);

            double re = 0.0;
            double im = 0.0;
            for (std::size_t n = 0; n < frames; ++n)
            {
                const double value = x[options.begin + n] * window[n];
                re += value * std::cos(phase[n]);
                im -= value * std::sin(phase[n]);
            }

            const double power = re * re + im * im;
            if (power > bestCarrier)
            {
                bestCarrier = power;
                bestOffset = offset;
            }
        }

        if (!(bestCarrier > 0.0))
            return result;

        const std::vector<double> phase = ReferencePhase(trackedHz, options.begin, frames, options.sampleRate, bestOffset);
        result.fitOffsetSamples = bestOffset;

        const double gain = CoherentGain(window);
        const double reference = static_cast<double>(frames) * gain / 2.0;
        result.carrierDbfs = DbFromPower(bestCarrier / (reference * reference));

        // Demodulated once: every bin below is read from this baseband, where the carrier sits at 0 Hz and the
        // sidebands at the multiples of the block rate either side of it.
        std::vector<Complex> baseband(frames);
        for (std::size_t n = 0; n < frames; ++n)
        {
            const double value = x[options.begin + n] * window[n];
            baseband[n] = Complex(value * std::cos(phase[n]), -value * std::sin(phase[n]));
        }

        const auto binPower = [&](std::ptrdiff_t k)
        {
            // One rotation per bin, carried along the window instead of a trigonometric call per sample.
            const Complex step = std::polar(1.0, kTwoPi * static_cast<double>(k) / static_cast<double>(frames));
            Complex rotation = 1.0;
            Complex bin{ 0.0, 0.0 };
            for (std::size_t n = 0; n < frames; ++n)
            {
                bin += baseband[n] * rotation;
                rotation *= step;
            }

            return std::norm(bin);
        };

        // Carrier and sidebands are both read as the energy summed over their bins. A tone spreads over the window's
        // main lobe, so its peak bin alone holds less than its energy (by the window's noise bandwidth): comparing a
        // summed sideband band against a peak carrier would overstate every sideband by that much.
        const auto firstBin = static_cast<std::ptrdiff_t>(std::max<std::size_t>(1, binOf(options.innerBlockRates * options.blockRateHz)));
        const auto lastBin = static_cast<std::ptrdiff_t>(binOf(static_cast<double>(options.orders) * options.blockRateHz));

        double carrierPower = 0.0;
        for (std::ptrdiff_t k = -(firstBin - 1); k <= firstBin - 1; ++k)
            carrierPower += binPower(k);

        // Every bin from the edge of the carrier's lobe out to the last order, once each side.
        double sidebandPower = 0.0;
        for (std::ptrdiff_t k = firstBin; k <= lastBin; ++k)
            sidebandPower += binPower(k) + binPower(-k);

        if (!(carrierPower > 0.0))
            return result;

        result.sidebandDbc = DbFromPower(sidebandPower / carrierPower);
        return result;
    }
} // namespace SparkyStudios::Audio::Amplitude::Fidelity