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

#include <Fidelity/Analysis/Null.h>

#include <algorithm>
#include <bit>
#include <cmath>

#include <Fidelity/Analysis/Fft.h>

namespace SparkyStudios::Audio::Amplitude::Fidelity
{
    NullResult AnalyzeNull(std::span<const double> a, std::span<const double> b, const NullOptions& options)
    {
        NullResult result;
        if (a.empty() || b.empty())
            return result;

        // c[m] = sum_n a[n + m] * b[n]; the peak lag m aligns a[n] with b[n - m].
        const std::size_t size = NextPowerOfTwo(a.size() + b.size());
        std::vector<Complex> fa(size, Complex(0.0, 0.0));
        std::vector<Complex> fb(size, Complex(0.0, 0.0));
        for (std::size_t i = 0; i < a.size(); ++i)
            fa[i] = Complex(a[i], 0.0);
        for (std::size_t i = 0; i < b.size(); ++i)
            fb[i] = Complex(b[i], 0.0);

        Fft(fa, false);
        Fft(fb, false);
        for (std::size_t k = 0; k < size; ++k)
            fa[k] *= std::conj(fb[k]);
        Fft(fa, true);

        const auto maxLag = static_cast<std::int64_t>(std::min(options.maxLag, size / 2 - 1));
        double best = fa[0].real();
        std::int64_t bestLag = 0;
        for (std::int64_t m = -maxLag; m <= maxLag; ++m)
        {
            const std::size_t index = m >= 0 ? static_cast<std::size_t>(m) : size - static_cast<std::size_t>(-m);
            if (fa[index].real() > best)
            {
                best = fa[index].real();
                bestLag = m;
            }
        }

        result.lag = bestLag;

        const auto aSize = static_cast<std::int64_t>(a.size());
        const auto bSize = static_cast<std::int64_t>(b.size());
        const auto guard = static_cast<std::int64_t>(options.guard);
        const std::int64_t first = std::max<std::int64_t>(0, bestLag) + guard;
        const std::int64_t last = std::min<std::int64_t>(aSize, bSize + bestLag) - guard;
        if (last <= first)
            return result;

        double ab = 0.0;
        double bb = 0.0;
        for (std::int64_t i = first; i < last; ++i)
        {
            const double bv = b[static_cast<std::size_t>(i - bestLag)];
            ab += a[static_cast<std::size_t>(i)] * bv;
            bb += bv * bv;
        }

        const double gain = options.matchGain && bb > 0.0 ? ab / bb : 1.0;
        result.gainDb = DbFromAmplitude(gain);

        result.residualBegin = static_cast<std::size_t>(first);
        result.residual.resize(static_cast<std::size_t>(last - first));

        double sumSquares = 0.0;
        double peak = 0.0;
        for (std::int64_t i = first; i < last; ++i)
        {
            const double r = a[static_cast<std::size_t>(i)] - gain * b[static_cast<std::size_t>(i - bestLag)];
            result.residual[static_cast<std::size_t>(i - first)] = r;
            sumSquares += r * r;
            peak = std::max(peak, std::abs(r));
        }

        result.residualRmsDbfs = DbFromAmplitude(std::sqrt(sumSquares / static_cast<double>(last - first)));
        result.residualPeakDbfs = DbFromAmplitude(peak);

        const std::size_t window = std::max<std::size_t>(1, options.windowSize);
        double worst = -1.0;
        for (std::size_t start = 0; start < result.residual.size(); start += window)
        {
            const std::size_t end = std::min(result.residual.size(), start + window);
            double windowPeak = 0.0;
            for (std::size_t i = start; i < end; ++i)
                windowPeak = std::max(windowPeak, std::abs(result.residual[i]));

            if (windowPeak > worst)
            {
                worst = windowPeak;
                result.worstWindowStart = result.residualBegin + start;
            }
        }

        return result;
    }

    std::size_t CountDifferences(std::span<const float> a, std::span<const float> b)
    {
        const std::size_t common = std::min(a.size(), b.size());
        std::size_t count = std::max(a.size(), b.size()) - common;
        for (std::size_t i = 0; i < common; ++i)
            if (std::bit_cast<std::uint32_t>(a[i]) != std::bit_cast<std::uint32_t>(b[i]))
                ++count;

        return count;
    }
} // namespace SparkyStudios::Audio::Amplitude::Fidelity
