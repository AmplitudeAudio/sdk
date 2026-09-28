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

#include <Fidelity/Analysis/Analytic.h>

#include <algorithm>
#include <cmath>
#include <numbers>

namespace SparkyStudios::Audio::Amplitude::Fidelity
{
    std::vector<Complex> AnalyticSignal(std::span<const double> x)
    {
        const std::size_t n = x.size();
        if (n == 0)
            return {};

        const std::size_t taper = std::min(kAnalyticTaper, n / 8);
        const std::size_t size = NextPowerOfTwo(2 * n);

        std::vector<Complex> spectrum(size, Complex(0.0, 0.0));
        for (std::size_t i = 0; i < n; ++i)
            spectrum[i] = Complex(x[i] * FadeGain(i, n, taper), 0.0);

        Fft(spectrum, false);

        // Keep DC and Nyquist, double the positive frequencies, drop the negative ones.
        for (std::size_t k = 1; k < size / 2; ++k)
            spectrum[k] *= 2.0;

        for (std::size_t k = size / 2 + 1; k < size; ++k)
            spectrum[k] = Complex(0.0, 0.0);

        Fft(spectrum, true);
        spectrum.resize(n);

        return spectrum;
    }

    Signal Envelope(std::span<const double> x)
    {
        const std::vector<Complex> analytic = AnalyticSignal(x);
        Signal envelope(analytic.size());
        for (std::size_t i = 0; i < analytic.size(); ++i)
            envelope[i] = std::abs(analytic[i]);

        return envelope;
    }

    void Unwrap(Signal& phase)
    {
        double offset = 0.0;
        for (std::size_t i = 1; i < phase.size(); ++i)
        {
            const double raw = phase[i] + offset;
            const double delta = raw - phase[i - 1];
            if (delta > std::numbers::pi)
                offset -= 2.0 * std::numbers::pi * std::round(delta / (2.0 * std::numbers::pi));
            else if (delta < -std::numbers::pi)
                offset += 2.0 * std::numbers::pi * std::round(-delta / (2.0 * std::numbers::pi));

            phase[i] = phase[i] + offset;
        }
    }

    Signal UnwrappedPhase(std::span<const double> x)
    {
        const std::vector<Complex> analytic = AnalyticSignal(x);
        Signal phase(analytic.size());
        for (std::size_t i = 0; i < analytic.size(); ++i)
            phase[i] = std::arg(analytic[i]);

        Unwrap(phase);
        return phase;
    }
} // namespace SparkyStudios::Audio::Amplitude::Fidelity
