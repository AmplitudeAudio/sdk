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

#include <Fidelity/Analysis/Pitch.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <numbers>

#include <Fidelity/Analysis/Analytic.h>

namespace SparkyStudios::Audio::Amplitude::Fidelity
{
    namespace
    {
        constexpr double kTwoPi = 2.0 * std::numbers::pi;

        double Cents(double frequency)
        {
            return 1200.0 * std::log2(std::max(frequency, 1e-9));
        }

        // Central-difference frequency of an unwrapped phase, then a centred moving average.
        Signal InstantaneousFrequency(const Signal& unwrapped, double sampleRate, std::size_t smoothing)
        {
            const std::size_t n = unwrapped.size();
            Signal raw(n, 0.0);
            for (std::size_t i = 1; i + 1 < n; ++i)
                raw[i] = (unwrapped[i + 1] - unwrapped[i - 1]) * sampleRate / (2.0 * kTwoPi);

            if (n > 1)
            {
                raw[0] = raw[1];
                raw[n - 1] = raw[n - 2];
            }

            return MovingAverage(raw, smoothing);
        }

        double QuantizationGrid(const Signal& frequency, std::size_t begin, std::size_t end, const PitchOptions& options)
        {
            std::vector<double> plateaus;
            std::size_t i = begin;
            while (i < end)
            {
                double lo = Cents(frequency[i]);
                double hi = lo;
                std::size_t j = i;
                while (j + 1 < end)
                {
                    const double c = Cents(frequency[j + 1]);
                    const double nextLo = std::min(lo, c);
                    const double nextHi = std::max(hi, c);
                    if (nextHi - nextLo > options.plateauToleranceCents)
                        break;

                    lo = nextLo;
                    hi = nextHi;
                    ++j;
                }

                if (j - i + 1 >= options.minPlateau)
                    plateaus.push_back(0.5 * (lo + hi));

                i = j + 1;
            }

            std::vector<double> distances;
            for (std::size_t k = 1; k < plateaus.size(); ++k)
            {
                const double d = std::abs(plateaus[k] - plateaus[k - 1]);
                if (d > options.plateauToleranceCents)
                    distances.push_back(d);
            }

            if (plateaus.size() < 3 || distances.empty())
                return 0.0;

            auto middle = distances.begin() + static_cast<std::ptrdiff_t>(distances.size() / 2);
            std::nth_element(distances.begin(), middle, distances.end());
            return *middle;
        }
    } // namespace

    PitchResult AnalyzePitch(std::span<const double> x, double sampleRate, const PitchOptions& options)
    {
        PitchResult result;
        const std::size_t n = x.size();
        const Signal phase = UnwrappedPhase(x);
        result.frequencyHz = InstantaneousFrequency(phase, sampleRate, options.smoothing);

        const std::size_t begin = options.begin;
        const std::size_t end = options.end == 0 ? n : std::min(options.end, n);
        const std::size_t span = options.smoothing;
        if (end <= begin + 2 * span)
            return result;

        const auto expectedAt = [&](std::size_t i)
        {
            return options.expectedHz.empty() ? options.constantExpectedHz : options.expectedHz[i];
        };

        double sumSquares = 0.0;
        std::size_t count = 0;
        for (std::size_t i = begin; i < end; ++i)
        {
            const double expected = expectedAt(i);
            if (expected <= 0.0)
                continue;

            const double deviation = CentsBetween(result.frequencyHz[i], expected);
            sumSquares += deviation * deviation;
            result.maxDeviationCents = std::max(result.maxDeviationCents, std::abs(deviation));
            ++count;
        }

        if (count > 0)
            result.rmsDeviationCents = std::sqrt(sumSquares / static_cast<double>(count));

        for (std::size_t i = begin + span; i + span < end; ++i)
        {
            const double measured = Cents(result.frequencyHz[i + span]) - Cents(result.frequencyHz[i - span]);
            const double expectedLo = expectedAt(i - span);
            const double expectedHi = expectedAt(i + span);
            const double expected = expectedLo > 0.0 && expectedHi > 0.0 ? Cents(expectedHi) - Cents(expectedLo) : 0.0;
            const double step = std::abs(measured - expected);
            if (step > result.largestStepCents)
            {
                result.largestStepCents = step;
                result.largestStepSample = i;
            }
        }

        result.quantizationGridCents = QuantizationGrid(result.frequencyHz, begin, end, options);

        // Phase jumps: the phase error against the integrated expected frequency should be smooth.
        if (expectedAt(begin) > 0.0)
        {
            Signal error(n, 0.0);
            double expectedPhase = phase[begin];
            for (std::size_t i = begin; i < end; ++i)
            {
                if (i > begin)
                    expectedPhase += kTwoPi * expectedAt(i) / sampleRate;

                error[i] = phase[i] - expectedPhase;
            }

            // Group over-threshold samples closer than `span`; report each group's largest second difference.
            bool inGroup = false;
            std::size_t groupPeak = 0;
            std::size_t groupLast = 0;
            double groupValue = 0.0;
            for (std::size_t i = begin + 1; i + 1 < end; ++i)
            {
                const double d2 = std::abs(error[i + 1] - 2.0 * error[i] + error[i - 1]);
                if (d2 <= options.phaseJumpRadians)
                    continue;

                if (inGroup && i - groupLast <= span)
                {
                    if (d2 > groupValue)
                    {
                        groupValue = d2;
                        groupPeak = i;
                    }

                    groupLast = i;
                    continue;
                }

                if (inGroup)
                    result.phaseJumps.push_back(groupPeak);

                inGroup = true;
                groupPeak = i;
                groupLast = i;
                groupValue = d2;
            }

            if (inGroup)
                result.phaseJumps.push_back(groupPeak);

            result.phaseJumpCount = result.phaseJumps.size();
        }

        return result;
    }

    double ChirpCycles(const ChirpModel& model, double seconds)
    {
        return model.f0Hz * seconds + 0.5 * model.rateHzPerSecond * seconds * seconds;
    }

    Signal EstimateChirpSourceTime(std::span<const double> x, double outputRate, const ChirpModel& model, double minLevel)
    {
        const std::size_t n = x.size();
        const std::vector<Complex> analytic = AnalyticSignal(x);

        Signal wrapped(n);
        for (std::size_t i = 0; i < n; ++i)
            wrapped[i] = std::arg(analytic[i]);

        Signal unwrapped = wrapped;
        Unwrap(unwrapped);
        const Signal frequency = InstantaneousFrequency(unwrapped, outputRate, 48);

        Signal tau(n, std::numeric_limits<double>::quiet_NaN());
        for (std::size_t i = 0; i < n; ++i)
        {
            if (std::abs(analytic[i]) < minLevel)
                continue;

            const double coarse = (frequency[i] - model.f0Hz) / model.rateHzPerSecond;
            const double expectedPhase = kTwoPi * ChirpCycles(model, coarse);
            // The analytic signal of sin(theta) has argument theta - pi / 2.
            const double measured = wrapped[i] + 0.5 * std::numbers::pi;
            const double turns = std::round((expectedPhase - measured) / kTwoPi);
            const double refinedPhase = measured + kTwoPi * turns;
            const double instantaneous = model.f0Hz + model.rateHzPerSecond * coarse;
            tau[i] = coarse + (refinedPhase - expectedPhase) / (kTwoPi * instantaneous);
        }

        return tau;
    }
} // namespace SparkyStudios::Audio::Amplitude::Fidelity
