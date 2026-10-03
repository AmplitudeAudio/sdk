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

#include <Fidelity/Analysis/Spectrum.h>

#include <algorithm>
#include <cmath>
#include <cstdint>

#include <Fidelity/Analysis/Fft.h>
#include <Fidelity/Analysis/Window.h>
#include <Fidelity/Signal.h>

namespace SparkyStudios::Audio::Amplitude::Fidelity
{
    PowerSpectrum AveragedPowerSpectrum(std::span<const double> x, double sampleRate, std::size_t frameSize)
    {
        PowerSpectrum spectrum;
        spectrum.sampleRate = sampleRate;

        std::size_t size = NextPowerOfTwo(std::max<std::size_t>(frameSize, 2));
        while (size > 2 && size > x.size())
            size >>= 1;

        spectrum.frameSize = size;
        spectrum.binHz = sampleRate / static_cast<double>(size);
        spectrum.power.assign(size / 2 + 1, 0.0);

        if (x.size() < size)
            return spectrum;

        const std::vector<double> window = BlackmanHarris7(size);
        double windowEnergy = 0.0;
        for (const double w : window)
            windowEnergy += w * w;

        const std::size_t hop = size / 2;
        const std::size_t frames = 1 + (x.size() - size) / hop;
        std::vector<Complex> buffer(size);

        for (std::size_t frame = 0; frame < frames; ++frame)
        {
            for (std::size_t i = 0; i < size; ++i)
                buffer[i] = Complex(x[frame * hop + i] * window[i], 0.0);

            Fft(buffer, false);

            for (std::size_t k = 0; k <= size / 2; ++k)
            {
                const double oneSided = k == 0 || k == size / 2 ? 1.0 : 2.0;
                spectrum.power[k] += oneSided * std::norm(buffer[k]) / (static_cast<double>(size) * windowEnergy);
            }
        }

        for (double& p : spectrum.power)
            p /= static_cast<double>(frames);

        return spectrum;
    }

    double BandPower(const PowerSpectrum& spectrum, double frequencyHz, std::size_t halfWidthBins)
    {
        if (spectrum.power.empty() || spectrum.binHz <= 0.0)
            return 0.0;

        const auto center = static_cast<std::int64_t>(std::llround(frequencyHz / spectrum.binHz));
        const auto last = static_cast<std::int64_t>(spectrum.power.size()) - 1;
        const auto half = static_cast<std::int64_t>(halfWidthBins);
        const std::int64_t lo = std::max<std::int64_t>(0, center - half);
        const std::int64_t hi = std::min<std::int64_t>(last, center + half);

        double sum = 0.0;
        for (std::int64_t k = lo; k <= hi; ++k)
            sum += spectrum.power[static_cast<std::size_t>(k)];

        return sum;
    }

    SpectrumResult AnalyzeSpectrum(std::span<const double> x, double sampleRate, const SpectrumOptions& options)
    {
        SpectrumResult result;
        const PowerSpectrum spectrum = AveragedPowerSpectrum(x, sampleRate, options.frameSize);
        if (spectrum.power.size() < 2)
            return result;

        const auto lobe = static_cast<std::int64_t>(options.mainLobeBins);
        const auto last = static_cast<std::int64_t>(spectrum.power.size()) - 1;
        const auto power = [&](std::int64_t k)
        {
            return spectrum.power[static_cast<std::size_t>(k)];
        };

        // The tone is searched near the nominal frequency: the engine's output is not guaranteed to sit exactly on it.
        const double searchLo = options.fundamentalHz * std::exp2(-options.searchCents / 1200.0);
        const double searchHi = options.fundamentalHz * std::exp2(options.searchCents / 1200.0);
        const auto kSearchLo = std::max<std::int64_t>(1, static_cast<std::int64_t>(std::floor(searchLo / spectrum.binHz)));
        const auto kSearchHi = std::min<std::int64_t>(last - 1, static_cast<std::int64_t>(std::ceil(searchHi / spectrum.binHz)));
        if (kSearchHi < kSearchLo)
            return result;

        std::int64_t peak = kSearchLo;
        for (std::int64_t k = kSearchLo; k <= kSearchHi; ++k)
            if (power(k) > power(peak))
                peak = k;

        if (power(peak) <= 0.0)
            return result;

        // Parabolic interpolation of the log power around the peak bin (the window's main lobe is close to Gaussian).
        double delta = 0.0;
        {
            const double a = std::log(std::max(power(peak - 1), 1e-300));
            const double b = std::log(power(peak));
            const double c = std::log(std::max(power(peak + 1), 1e-300));
            const double denominator = a - 2.0 * b + c;
            if (denominator < 0.0)
                delta = std::clamp(0.5 * (a - c) / denominator, -0.5, 0.5);
        }

        result.measuredFundamentalHz = (static_cast<double>(peak) + delta) * spectrum.binHz;
        result.frequencyErrorCents = CentsBetween(result.measuredFundamentalHz, options.fundamentalHz);
        const double f0 = result.measuredFundamentalHz;

        const double bandHigh = std::min(options.bandHighHz, static_cast<double>(last - lobe) * spectrum.binHz);
        const auto kLow = static_cast<std::int64_t>(std::ceil(options.bandLowHz / spectrum.binHz));
        const auto kHigh = static_cast<std::int64_t>(std::floor(bandHigh / spectrum.binHz));

        std::vector<bool> masked(spectrum.power.size(), false);
        const auto mask = [&](std::int64_t center)
        {
            for (std::int64_t k = std::max<std::int64_t>(0, center - lobe); k <= std::min(last, center + lobe); ++k)
                masked[static_cast<std::size_t>(k)] = true;
        };

        double fundamental = 0.0;
        for (std::int64_t k = std::max<std::int64_t>(0, peak - lobe); k <= std::min(last, peak + lobe); ++k)
            fundamental += power(k);

        result.fundamentalDbfs = DbFromPower(fundamental / 0.5);
        mask(peak);

        double harmonicPower = 0.0;
        for (std::size_t h = 2; h <= options.harmonics; ++h)
        {
            const double frequency = f0 * static_cast<double>(h);
            if (frequency > bandHigh)
                break;

            harmonicPower += BandPower(spectrum, frequency, options.mainLobeBins);
            mask(static_cast<std::int64_t>(std::llround(frequency / spectrum.binHz)));
        }

        result.thdDb = DbFromPower(harmonicPower / fundamental);

        double residual = 0.0;
        std::vector<double> floorBins;
        for (std::int64_t k = kLow; k <= kHigh; ++k)
        {
            if (std::llabs(k - peak) > lobe)
                residual += power(k);

            if (!masked[static_cast<std::size_t>(k)])
                floorBins.push_back(power(k));
        }

        result.thdnDb = DbFromPower(residual / fundamental);
        result.sinadDb = -result.thdnDb;

        // Worst spur: the loudest unmasked in-band bin, summed over its own lobe without the masked bins.
        std::int64_t spurBin = -1;
        for (std::int64_t k = kLow; k <= kHigh; ++k)
            if (!masked[static_cast<std::size_t>(k)] && (spurBin < 0 || power(k) > power(spurBin)))
                spurBin = k;

        if (spurBin >= 0)
        {
            double spurPower = 0.0;
            for (std::int64_t k = std::max<std::int64_t>(0, spurBin - lobe); k <= std::min(last, spurBin + lobe); ++k)
                if (!masked[static_cast<std::size_t>(k)])
                    spurPower += power(k);

            result.worstSpurDbc = DbFromPower(spurPower / fundamental);
            result.worstSpurHz = static_cast<double>(spurBin) * spectrum.binHz;
        }

        if (!floorBins.empty())
        {
            auto middle = floorBins.begin() + static_cast<std::ptrdiff_t>(floorBins.size() / 2);
            std::nth_element(floorBins.begin(), middle, floorBins.end());
            result.noiseFloorDbfs = DbFromPower(*middle / 0.5);
        }

        return result;
    }
} // namespace SparkyStudios::Audio::Amplitude::Fidelity
