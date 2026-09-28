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

#include <Fidelity/Signal.h>

#include <algorithm>
#include <cmath>
#include <numbers>

namespace SparkyStudios::Audio::Amplitude::Fidelity
{
    double DbFromAmplitude(double amplitude)
    {
        return 20.0 * std::log10(std::max(std::abs(amplitude), kMinAmplitude));
    }

    double DbFromPower(double power)
    {
        return 10.0 * std::log10(std::max(power, kMinAmplitude * kMinAmplitude));
    }

    double AmplitudeFromDb(double db)
    {
        return std::pow(10.0, db / 20.0);
    }

    double CentsBetween(double frequency, double reference)
    {
        return 1200.0 * std::log2(frequency / reference);
    }

    double Rms(std::span<const double> x)
    {
        if (x.empty())
            return 0.0;

        double sum = 0.0;
        for (const double v : x)
            sum += v * v;

        return std::sqrt(sum / static_cast<double>(x.size()));
    }

    double Peak(std::span<const double> x)
    {
        double peak = 0.0;
        for (const double v : x)
            peak = std::max(peak, std::abs(v));

        return peak;
    }

    Signal ToSignal(std::span<const float> x)
    {
        return Signal(x.begin(), x.end());
    }

    std::vector<float> ToFloats(std::span<const double> x)
    {
        std::vector<float> out(x.size());
        for (std::size_t i = 0; i < x.size(); ++i)
            out[i] = static_cast<float>(x[i]);

        return out;
    }

    Signal MakeSine(std::size_t length, double sampleRate, double frequency, double amplitude, double phase)
    {
        Signal x(length);
        for (std::size_t i = 0; i < length; ++i)
        {
            const double cycles = std::fmod(static_cast<double>(i) * frequency / sampleRate, 1.0);
            x[i] = amplitude * std::sin(2.0 * std::numbers::pi * cycles + phase);
        }

        return x;
    }

    double FadeGain(std::size_t n, std::size_t total, std::size_t fadeLength)
    {
        if (fadeLength == 0)
            return 1.0;

        if (n < fadeLength)
            return 0.5 - 0.5 * std::cos(std::numbers::pi * static_cast<double>(n) / static_cast<double>(fadeLength));

        if (n + fadeLength >= total)
            return 0.5 - 0.5 * std::cos(std::numbers::pi * static_cast<double>(total - 1 - n) / static_cast<double>(fadeLength));

        return 1.0;
    }

    void ApplyFades(Signal& x, std::size_t fadeLength)
    {
        for (std::size_t i = 0; i < x.size(); ++i)
            x[i] *= FadeGain(i, x.size(), fadeLength);
    }

    Signal MovingAverage(std::span<const double> x, std::size_t window)
    {
        const std::size_t n = x.size();
        Signal out(n, 0.0);
        if (n == 0 || window == 0)
            return out;

        std::vector<double> prefix(n + 1, 0.0);
        for (std::size_t i = 0; i < n; ++i)
            prefix[i + 1] = prefix[i] + x[i];

        const std::size_t half = window / 2;
        for (std::size_t i = 0; i < n; ++i)
        {
            const std::size_t lo = i >= half ? i - half : 0;
            const std::size_t hi = std::min(n, i + (window - half));
            out[i] = (prefix[hi] - prefix[lo]) / static_cast<double>(hi - lo);
        }

        return out;
    }

    Random::Random(std::uint64_t seed)
        : _engine(seed)
    {}

    double Random::Uniform()
    {
        return static_cast<double>(_engine() >> 11) * 0x1.0p-53;
    }

    double Random::Symmetric()
    {
        return 2.0 * Uniform() - 1.0;
    }
} // namespace SparkyStudios::Audio::Amplitude::Fidelity
