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
        const double bandHigh = std::min(options.bandHighHz, static_cast<double>(last - lobe) * spectrum.binHz);
        const auto kLow = static_cast<std::int64_t>(std::ceil(options.bandLowHz / spectrum.binHz));
        const auto kHigh = static_cast<std::int64_t>(std::floor(bandHigh / spectrum.binHz));

        std::vector<bool> masked(spectrum.power.size(), false);
        const auto mask = [&](double frequency)
        {
            const auto center = static_cast<std::int64_t>(std::llround(frequency / spectrum.binHz));
            for (std::int64_t k = std::max<std::int64_t>(0, center - lobe); k <= std::min(last, center + lobe); ++k)
                masked[static_cast<std::size_t>(k)] = true;
        };

        const double fundamental = BandPower(spectrum, options.fundamentalHz, options.mainLobeBins);
        if (fundamental <= 0.0)
            return result;

        result.fundamentalDbfs = DbFromPower(fundamental / 0.5);
        mask(options.fundamentalHz);

        double harmonicPower = 0.0;
        for (std::size_t h = 2; h <= options.harmonics; ++h)
        {
            const double frequency = options.fundamentalHz * static_cast<double>(h);
            if (frequency > bandHigh)
                break;

            harmonicPower += BandPower(spectrum, frequency, options.mainLobeBins);
            mask(frequency);
        }

        result.thdDb = DbFromPower(harmonicPower / fundamental);

        const auto fundamentalBin = static_cast<std::int64_t>(std::llround(options.fundamentalHz / spectrum.binHz));
        double residual = 0.0;
        std::vector<double> floorBins;
        for (std::int64_t k = kLow; k <= kHigh; ++k)
        {
            const double p = spectrum.power[static_cast<std::size_t>(k)];
            if (std::llabs(k - fundamentalBin) > lobe)
                residual += p;

            if (!masked[static_cast<std::size_t>(k)])
                floorBins.push_back(p);
        }

        result.thdnDb = DbFromPower(residual / fundamental);
        result.sinadDb = -result.thdnDb;

        std::int64_t spurBin = -1;
        double spurPeak = -1.0;
        for (std::int64_t k = kLow; k <= kHigh; ++k)
        {
            const double p = spectrum.power[static_cast<std::size_t>(k)];
            if (!masked[static_cast<std::size_t>(k)] && p > spurPeak)
            {
                spurPeak = p;
                spurBin = k;
            }
        }

        if (spurBin >= 0)
        {
            const double spurHz = static_cast<double>(spurBin) * spectrum.binHz;
            result.worstSpurDbc = DbFromPower(BandPower(spectrum, spurHz, options.mainLobeBins) / fundamental);
            result.worstSpurHz = spurHz;
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
