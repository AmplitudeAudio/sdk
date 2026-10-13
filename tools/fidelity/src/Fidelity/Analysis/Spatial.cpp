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

#include <Fidelity/Analysis/Spatial.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>

namespace SparkyStudios::Audio::Amplitude::Fidelity
{
    namespace
    {
        void LargestStep(const std::vector<double>& values, double& largest, std::size_t& window)
        {
            for (std::size_t w = 1; w < values.size(); ++w)
            {
                if (!std::isfinite(values[w]) || !std::isfinite(values[w - 1]))
                    continue;

                const double step = std::abs(values[w] - values[w - 1]);
                if (step > largest)
                {
                    largest = step;
                    window = w;
                }
            }
        }
    } // namespace

    SpatialResult AnalyzeSpatial(
        std::span<const double> left, std::span<const double> right, double sampleRate, const SpatialOptions& options)
    {
        SpatialResult result;
        const std::size_t n = std::min(left.size(), right.size());
        const std::size_t begin = options.begin;
        const std::size_t end = options.end == 0 ? n : std::min(options.end, n);
        const std::size_t window = std::max<std::size_t>(1, options.window);
        const auto maxLag = static_cast<std::int64_t>(std::llround(options.maxItdSeconds * sampleRate));
        constexpr double kNaN = std::numeric_limits<double>::quiet_NaN();

        std::vector<double> correlation(static_cast<std::size_t>(2 * maxLag + 1));
        for (std::size_t start = begin; start + window <= end; start += window)
        {
            double energyLeft = 0.0;
            double energyRight = 0.0;
            for (std::size_t i = start; i < start + window; ++i)
            {
                energyLeft += left[i] * left[i];
                energyRight += right[i] * right[i];
            }

            if (energyLeft <= 0.0 || energyRight <= 0.0)
            {
                result.ildDb.push_back(kNaN);
                result.itdSamples.push_back(kNaN);
                continue;
            }

            result.ildDb.push_back(10.0 * std::log10(energyLeft / energyRight));

            // c(d) = sum_n left[n - d] * right[n].
            std::size_t best = 0;
            for (std::int64_t d = -maxLag; d <= maxLag; ++d)
            {
                double sum = 0.0;
                for (std::size_t i = start; i < start + window; ++i)
                {
                    const std::int64_t index = static_cast<std::int64_t>(i) - d;
                    if (index >= 0 && index < static_cast<std::int64_t>(n))
                        sum += left[static_cast<std::size_t>(index)] * right[i];
                }

                const auto slot = static_cast<std::size_t>(d + maxLag);
                correlation[slot] = sum;
                if (correlation[slot] > correlation[best])
                    best = slot;
            }

            double delta = 0.0;
            if (best > 0 && best + 1 < correlation.size())
            {
                const double y0 = correlation[best - 1];
                const double y1 = correlation[best];
                const double y2 = correlation[best + 1];
                const double denominator = y0 - 2.0 * y1 + y2;
                if (denominator != 0.0)
                    delta = 0.5 * (y0 - y2) / denominator;
            }

            result.itdSamples.push_back(static_cast<double>(static_cast<std::int64_t>(best) - maxLag) + delta);
        }

        LargestStep(result.ildDb, result.largestIldStepDb, result.largestIldStepWindow);
        LargestStep(result.itdSamples, result.largestItdStepSamples, result.largestItdStepWindow);
        return result;
    }
} // namespace SparkyStudios::Audio::Amplitude::Fidelity
