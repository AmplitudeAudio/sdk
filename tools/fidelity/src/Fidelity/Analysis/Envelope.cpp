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

#include <Fidelity/Analysis/Envelope.h>

#include <algorithm>
#include <cmath>

#include <Fidelity/Analysis/Analytic.h>
#include <Fidelity/Analysis/Spectrum.h>

namespace SparkyStudios::Audio::Amplitude::Fidelity
{
    namespace
    {
        void MeasureSidebands(std::span<const double> region, double sampleRate, const EnvelopeOptions& options, EnvelopeResult& result)
        {
            const PowerSpectrum spectrum = AveragedPowerSpectrum(region, sampleRate, 32768);
            const double carrier = BandPower(spectrum, options.carrierHz, 8);
            if (carrier <= 0.0)
                return;

            const double guard = 16.0 * spectrum.binHz;
            for (const double rate : options.updateRatesHz)
            {
                for (std::size_t order = 1; order <= options.sidebandOrders; ++order)
                {
                    for (const double sign : { -1.0, 1.0 })
                    {
                        const double frequency = options.carrierHz + sign * static_cast<double>(order) * rate;
                        if (frequency <= guard || frequency >= sampleRate / 2.0 - guard)
                            continue;

                        // Too close to the carrier to separate from its main lobe.
                        if (std::abs(frequency - options.carrierHz) <= guard)
                            continue;

                        Sideband sideband;
                        sideband.rateHz = rate;
                        sideband.order = order;
                        sideband.frequencyHz = frequency;
                        sideband.levelDbc = DbFromPower(BandPower(spectrum, frequency, 8) / carrier);
                        result.worstSidebandDbc = std::max(result.worstSidebandDbc, sideband.levelDbc);
                        result.sidebands.push_back(sideband);
                    }
                }
            }
        }
    } // namespace

    EnvelopeResult AnalyzeEnvelope(std::span<const double> x, double sampleRate, const EnvelopeOptions& options)
    {
        EnvelopeResult result;
        result.envelope = Envelope(x);

        const std::size_t n = x.size();
        const std::size_t begin = options.begin;
        const std::size_t end = options.end == 0 ? n : std::min(options.end, n);
        const std::size_t span = options.stepSpan;
        if (end <= begin + 2 * span)
            return result;

        const Signal reference = options.expected.empty() ? MovingAverage(result.envelope, options.smoothWindow) : options.expected;
        const double minLevel = AmplitudeFromDb(options.minLevelDbfs);

        Signal deviationDb(n, 0.0);
        std::vector<bool> valid(n, false);
        double sumSquares = 0.0;
        double sumSquaresDb = 0.0;
        std::size_t count = 0;
        for (std::size_t i = begin; i < end; ++i)
        {
            if (reference[i] < minLevel)
                continue;

            const double ratio = result.envelope[i] / reference[i];
            const double db = DbFromAmplitude(ratio);
            sumSquares += (ratio - 1.0) * (ratio - 1.0);
            sumSquaresDb += db * db;
            result.maxErrorDb = std::max(result.maxErrorDb, std::abs(db));
            deviationDb[i] = db;
            valid[i] = true;
            ++count;
        }

        if (count > 0)
        {
            result.modulationNoiseDbc = DbFromAmplitude(std::sqrt(sumSquares / static_cast<double>(count)));
            result.rmsErrorDb = std::sqrt(sumSquaresDb / static_cast<double>(count));
        }

        Signal step(n, 0.0);
        for (std::size_t i = begin + span; i + span < end; ++i)
        {
            if (!valid[i - span] || !valid[i + span])
                continue;

            step[i] = std::abs(deviationDb[i + span] - deviationDb[i - span]);
            if (step[i] > result.largestStepDb)
            {
                result.largestStepDb = step[i];
                result.largestStepSample = i;
            }
        }

        // Count local maxima above the threshold, at most one per 2 * span samples.
        const std::size_t separation = 2 * span;
        bool counted = false;
        std::size_t lastCounted = 0;
        for (std::size_t i = begin + span; i + span < end; ++i)
        {
            if (step[i] < options.stepThresholdDb)
                continue;

            const std::size_t lo = i >= separation ? i - separation : 0;
            const std::size_t hi = std::min(n, i + separation + 1);
            bool isMaximum = true;
            for (std::size_t j = lo; j < hi && isMaximum; ++j)
                if (step[j] > step[i] || (step[j] == step[i] && j < i))
                    isMaximum = false;

            if (!isMaximum || (counted && i - lastCounted <= separation))
                continue;

            ++result.stepCount;
            lastCounted = i;
            counted = true;
        }

        if (!options.updateRatesHz.empty())
            MeasureSidebands(x.subspan(begin, end - begin), sampleRate, options, result);

        return result;
    }

    double CrossingTime(std::span<const double> envelope, double level, double searchBegin, bool rising)
    {
        const auto start = static_cast<std::size_t>(std::max(1.0, std::ceil(searchBegin)));
        for (std::size_t i = start; i < envelope.size(); ++i)
        {
            const double a = envelope[i - 1];
            const double b = envelope[i];
            const bool crossed = rising ? (a < level && b >= level) : (a > level && b <= level);
            if (crossed)
                return static_cast<double>(i - 1) + (level - a) / (b - a);
        }

        return -1.0;
    }

    Signal TimingEnvelope(std::span<const double> x)
    {
        return MovingAverage(Envelope(x), kTimingSmoothing);
    }

    double PlateauLevel(std::span<const double> envelope, std::size_t begin, std::size_t end)
    {
        end = std::min(end, envelope.size());
        if (end <= begin)
            return 0.0;

        std::vector<double> values(
            envelope.begin() + static_cast<std::ptrdiff_t>(begin), envelope.begin() + static_cast<std::ptrdiff_t>(end));
        auto middle = values.begin() + static_cast<std::ptrdiff_t>(values.size() / 2);
        std::nth_element(values.begin(), middle, values.end());
        return *middle;
    }

    double SustainedCrossing(std::span<const double> envelope, double level, double searchBegin, bool rising, std::size_t hold)
    {
        double from = searchBegin;
        while (true)
        {
            const double crossing = CrossingTime(envelope, level, from, rising);
            if (crossing < 0.0)
                return -1.0;

            const auto first = static_cast<std::size_t>(std::ceil(crossing));
            const std::size_t last = std::min(envelope.size(), first + hold);
            bool held = true;
            for (std::size_t i = first; i < last && held; ++i)
                held = rising ? envelope[i] >= level : envelope[i] <= level;

            if (held)
                return crossing;

            from = static_cast<double>(first);
        }
    }

    double LastFallingCrossing(std::span<const double> envelope, double level)
    {
        for (std::size_t i = envelope.size(); i-- > 1;)
        {
            const double a = envelope[i - 1];
            const double b = envelope[i];
            if (a > level && b <= level)
                return static_cast<double>(i - 1) + (level - a) / (b - a);
        }

        return -1.0;
    }

    FadeTiming MeasureFade(std::span<const double> envelope, double sampleRate, double from, double to, double searchBegin)
    {
        FadeTiming timing;
        const bool rising = to > from;
        const double level10 = from + 0.1 * (to - from);
        const double level90 = from + 0.9 * (to - from);

        const double start = CrossingTime(envelope, level10, searchBegin, rising);
        if (start < 0.0)
            return timing;

        const double end = CrossingTime(envelope, level90, start, rising);
        if (end < 0.0)
            return timing;

        timing.found = true;
        timing.start = start;
        timing.end = end;
        timing.durationMs = (end - start) / sampleRate * 1000.0;
        return timing;
    }
} // namespace SparkyStudios::Audio::Amplitude::Fidelity
