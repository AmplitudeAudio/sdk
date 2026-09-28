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

#include <Fidelity/Analysis/FrequencyResponse.h>

#include <algorithm>
#include <cmath>
#include <numbers>

#include <Fidelity/Analysis/Fft.h>

namespace SparkyStudios::Audio::Amplitude::Fidelity
{
    namespace
    {
        // The inverse filter: time-reversed sweep with a -6 dB/octave envelope correction.
        Signal InverseFilter(const Signal& sweep, double sampleRate, const SweepModel& model)
        {
            const std::size_t n = sweep.size();
            const double rate = std::log(model.f2Hz / model.f1Hz);
            Signal inverse(n);
            for (std::size_t i = 0; i < n; ++i)
            {
                const double t = static_cast<double>(i) / sampleRate;
                inverse[i] = sweep[n - 1 - i] * std::exp(-t * rate / model.durationSeconds);
            }

            return inverse;
        }

        Signal Convolve(std::span<const double> a, std::span<const double> b)
        {
            const std::size_t length = a.size() + b.size() - 1;
            const std::size_t size = NextPowerOfTwo(length);
            std::vector<Complex> fa(size, Complex(0.0, 0.0));
            std::vector<Complex> fb(size, Complex(0.0, 0.0));
            for (std::size_t i = 0; i < a.size(); ++i)
                fa[i] = Complex(a[i], 0.0);
            for (std::size_t i = 0; i < b.size(); ++i)
                fb[i] = Complex(b[i], 0.0);

            Fft(fa, false);
            Fft(fb, false);
            for (std::size_t k = 0; k < size; ++k)
                fa[k] *= fb[k];
            Fft(fa, true);

            Signal out(length);
            for (std::size_t i = 0; i < length; ++i)
                out[i] = fa[i].real();

            return out;
        }

        // Spectrum of the impulse response around its peak: 1/8 of the window before it (faded in), the last quarter faded out.
        std::vector<Complex> WindowedIrSpectrum(const Signal& ir, std::size_t length)
        {
            std::size_t peak = 0;
            for (std::size_t i = 1; i < ir.size(); ++i)
                if (std::abs(ir[i]) > std::abs(ir[peak]))
                    peak = i;

            const std::size_t pre = length / 8;
            const std::size_t tail = length / 4;
            const std::size_t start = peak >= pre ? peak - pre : 0;

            std::vector<Complex> buffer(length, Complex(0.0, 0.0));
            for (std::size_t i = 0; i < length; ++i)
            {
                const std::size_t index = start + i;
                const double value = index < ir.size() ? ir[index] : 0.0;

                double weight = 1.0;
                if (i < pre)
                    weight = 0.5 - 0.5 * std::cos(std::numbers::pi * static_cast<double>(i) / static_cast<double>(pre));
                else if (i >= length - tail)
                    weight = 0.5 + 0.5 * std::cos(std::numbers::pi * static_cast<double>(i - (length - tail)) / static_cast<double>(tail));

                buffer[i] = Complex(value * weight, 0.0);
            }

            Fft(buffer, false);
            return buffer;
        }
    } // namespace

    Signal RenderLogSweep(const SweepModel& model, double sampleRate, double maxFrequencyHz)
    {
        const auto n = static_cast<std::size_t>(std::llround(model.durationSeconds * sampleRate));
        const double rate = std::log(model.f2Hz / model.f1Hz);
        const double scale = 2.0 * std::numbers::pi * model.f1Hz * model.durationSeconds / rate;

        Signal x(n);
        for (std::size_t i = 0; i < n; ++i)
        {
            const double t = static_cast<double>(i) / sampleRate;
            x[i] = model.amplitude * std::sin(scale * (std::exp(t * rate / model.durationSeconds) - 1.0));
        }

        const auto fade = static_cast<std::size_t>(std::llround(model.fadeSeconds * sampleRate));
        ApplyFades(x, fade);

        if (maxFrequencyHz < model.f2Hz)
        {
            // Instantaneous frequency f1 exp(t R / T) reaches maxFrequencyHz at this time.
            const double cutSeconds = model.durationSeconds * std::log(maxFrequencyHz / model.f1Hz) / rate;
            const auto cut = std::min(n, static_cast<std::size_t>(std::llround(cutSeconds * sampleRate)));
            for (std::size_t i = 0; i < n; ++i)
            {
                if (i >= cut)
                    x[i] = 0.0;
                else if (i + fade >= cut)
                    x[i] *= 0.5 -
                        0.5 *
                            std::cos(std::numbers::pi * static_cast<double>(cut - i) / static_cast<double>(std::max<std::size_t>(fade, 1)));
            }
        }

        return x;
    }

    ResponseResult AnalyzeFrequencyResponse(
        std::span<const double> captured, double sampleRate, const SweepModel& model, const ResponseOptions& options)
    {
        ResponseResult result;
        const double maxFrequency = std::min(model.f2Hz, 0.95 * sampleRate / 2.0);
        const Signal reference = RenderLogSweep(model, sampleRate, maxFrequency);
        const Signal inverse = InverseFilter(reference, sampleRate, model);

        const std::vector<Complex> measured = WindowedIrSpectrum(Convolve(captured, inverse), options.irLength);
        const std::vector<Complex> ideal = WindowedIrSpectrum(Convolve(reference, inverse), options.irLength);

        const double binHz = sampleRate / static_cast<double>(options.irLength);
        for (std::size_t k = 1; k <= options.irLength / 2; ++k)
        {
            const double frequency = static_cast<double>(k) * binHz;
            if (frequency < 2.0 * model.f1Hz || frequency > maxFrequency)
                continue;

            const double magnitude = std::abs(measured[k]) / std::max(std::abs(ideal[k]), 1e-30);
            result.frequencyHz.push_back(frequency);
            result.magnitudeDb.push_back(20.0 * std::log10(std::max(magnitude, 1e-20)));
        }

        if (result.frequencyHz.empty())
            return result;

        std::size_t referenceIndex = 0;
        for (std::size_t i = 1; i < result.frequencyHz.size(); ++i)
            if (std::abs(result.frequencyHz[i] - options.referenceHz) < std::abs(result.frequencyHz[referenceIndex] - options.referenceHz))
                referenceIndex = i;

        const double offset = result.magnitudeDb[referenceIndex];
        for (double& value : result.magnitudeDb)
            value -= offset;

        double lo = 0.0;
        double hi = 0.0;
        bool any = false;
        const double passLow = std::max(options.passbandLowHz, 2.0 * model.f1Hz);
        for (std::size_t i = 0; i < result.frequencyHz.size(); ++i)
        {
            if (result.frequencyHz[i] < passLow || result.frequencyHz[i] > options.passbandHighHz)
                continue;

            lo = any ? std::min(lo, result.magnitudeDb[i]) : result.magnitudeDb[i];
            hi = any ? std::max(hi, result.magnitudeDb[i]) : result.magnitudeDb[i];
            any = true;
        }

        result.rippleDb = any ? hi - lo : 0.0;

        result.minus3dBHz = result.frequencyHz.back();
        for (std::size_t i = referenceIndex; i < result.frequencyHz.size(); ++i)
        {
            if (result.magnitudeDb[i] < -3.0)
            {
                result.minus3dBHz = result.frequencyHz[i];
                break;
            }
        }

        return result;
    }
} // namespace SparkyStudios::Audio::Amplitude::Fidelity
