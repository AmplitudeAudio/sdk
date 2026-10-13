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

#include <Fidelity/Analysis/Click.h>

#include <algorithm>
#include <cmath>
#include <numbers>

#include <Fidelity/Analysis/Fft.h>

namespace SparkyStudios::Audio::Amplitude::Fidelity
{
    Signal HighPass(std::span<const double> x, double sampleRate, double cutoffHz, double transitionHz)
    {
        const std::size_t n = x.size();
        if (n == 0)
            return {};

        // Zero-pad to twice the length so the circular wrap cannot fold one end onto the other.
        const std::size_t size = NextPowerOfTwo(2 * n);
        std::vector<Complex> spectrum(size, Complex(0.0, 0.0));
        for (std::size_t i = 0; i < n; ++i)
            spectrum[i] = Complex(x[i], 0.0);

        Fft(spectrum, false);

        const double low = std::max(0.0, cutoffHz - transitionHz);
        for (std::size_t k = 0; k < size; ++k)
        {
            const std::size_t bin = k <= size / 2 ? k : size - k;
            const double frequency = static_cast<double>(bin) * sampleRate / static_cast<double>(size);

            double gain = 1.0;
            if (frequency <= low)
                gain = 0.0;
            else if (frequency < cutoffHz)
                gain = 0.5 - 0.5 * std::cos(std::numbers::pi * (frequency - low) / (cutoffHz - low));

            spectrum[k] *= gain;
        }

        Fft(spectrum, true);

        Signal band(n);
        for (std::size_t i = 0; i < n; ++i)
            band[i] = spectrum[i].real();

        return band;
    }

    ClickResult AnalyzeClicks(std::span<const double> x, double sampleRate, const ClickOptions& options)
    {
        ClickResult result;
        result.band = HighPass(x, sampleRate, options.highPassHz, options.transitionHz);

        const std::size_t n = x.size();
        if (n <= 2 * options.guard)
            return result;

        const std::size_t begin = options.guard;
        const std::size_t end = n - options.guard;

        Signal magnitude(n);
        for (std::size_t i = 0; i < n; ++i)
            magnitude[i] = std::abs(result.band[i]);

        {
            std::vector<double> region(
                magnitude.begin() + static_cast<std::ptrdiff_t>(begin), magnitude.begin() + static_cast<std::ptrdiff_t>(end));
            result.maxPeakDbfs = DbFromAmplitude(*std::max_element(region.begin(), region.end()));
            auto middle = region.begin() + static_cast<std::ptrdiff_t>(region.size() / 2);
            std::nth_element(region.begin(), middle, region.end());
            result.floorDbfs = DbFromAmplitude(*middle);
        }

        const double minLevel = AmplitudeFromDb(options.minEventDbfs);
        const double margin = AmplitudeFromDb(options.marginDb);
        const std::size_t separation = options.minSeparation;
        const std::size_t half = options.floorWindow / 2;

        std::vector<ClickEvent> candidates;
        std::vector<double> scratch;
        for (std::size_t i = begin; i < end; ++i)
        {
            const double value = magnitude[i];
            if (value < minLevel)
                continue;

            // Local maximum within ±separation; the earliest sample wins a tie.
            const std::size_t lo = i >= separation ? i - separation : 0;
            const std::size_t hi = std::min(n, i + separation + 1);
            bool isMaximum = true;
            for (std::size_t j = lo; j < hi && isMaximum; ++j)
                if (magnitude[j] > value || (magnitude[j] == value && j < i))
                    isMaximum = false;

            if (!isMaximum)
                continue;

            const std::size_t floorBegin = i >= half ? i - half : 0;
            const std::size_t floorEnd = std::min(n, i + half);
            scratch.assign(
                magnitude.begin() + static_cast<std::ptrdiff_t>(floorBegin), magnitude.begin() + static_cast<std::ptrdiff_t>(floorEnd));
            auto middle = scratch.begin() + static_cast<std::ptrdiff_t>(scratch.size() / 2);
            std::nth_element(scratch.begin(), middle, scratch.end());
            const double floor = *middle;

            if (value < floor * margin)
                continue;

            candidates.push_back({ i, DbFromAmplitude(value), DbFromAmplitude(value) - DbFromAmplitude(floor) });
        }

        // Merge ringing: consecutive candidates closer than mergeWindow keep only the loudest.
        for (const ClickEvent& candidate : candidates)
        {
            if (!result.events.empty() && candidate.sample - result.events.back().sample < options.mergeWindow)
            {
                if (candidate.peakDbfs > result.events.back().peakDbfs)
                    result.events.back() = candidate;

                continue;
            }

            result.events.push_back(candidate);
        }

        return result;
    }
} // namespace SparkyStudios::Audio::Amplitude::Fidelity
