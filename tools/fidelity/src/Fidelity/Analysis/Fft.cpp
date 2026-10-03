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

#include <Fidelity/Analysis/Fft.h>

#include <cassert>
#include <numbers>
#include <utility>

namespace SparkyStudios::Audio::Amplitude::Fidelity
{
    bool IsPowerOfTwo(std::size_t n)
    {
        return n != 0 && (n & (n - 1)) == 0;
    }

    std::size_t NextPowerOfTwo(std::size_t n)
    {
        std::size_t power = 1;
        while (power < n)
            power <<= 1;

        return power;
    }

    void Fft(std::vector<Complex>& data, bool inverse)
    {
        const std::size_t n = data.size();
        assert(IsPowerOfTwo(n));

        if (n < 2)
            return;

        // Bit-reversal permutation.
        for (std::size_t i = 1, j = 0; i < n; ++i)
        {
            std::size_t bit = n >> 1;
            for (; (j & bit) != 0; bit >>= 1)
                j ^= bit;

            j ^= bit;

            if (i < j)
                std::swap(data[i], data[j]);
        }

        // Twiddles for the full size, each computed directly (no recurrence) to keep the error at machine precision.
        const double sign = inverse ? 1.0 : -1.0;
        std::vector<Complex> twiddles(n / 2);
        for (std::size_t k = 0; k < n / 2; ++k)
            twiddles[k] = std::polar(1.0, sign * 2.0 * std::numbers::pi * static_cast<double>(k) / static_cast<double>(n));

        for (std::size_t length = 2; length <= n; length <<= 1)
        {
            const std::size_t half = length / 2;
            const std::size_t stride = n / length;

            for (std::size_t start = 0; start < n; start += length)
            {
                for (std::size_t k = 0; k < half; ++k)
                {
                    const Complex t = twiddles[k * stride] * data[start + k + half];
                    const Complex u = data[start + k];
                    data[start + k] = u + t;
                    data[start + k + half] = u - t;
                }
            }
        }

        if (inverse)
        {
            const double scale = 1.0 / static_cast<double>(n);
            for (auto& value : data)
                value *= scale;
        }
    }

    std::vector<Complex> NaiveDft(const std::vector<Complex>& data, bool inverse)
    {
        const std::size_t n = data.size();
        const double sign = inverse ? 1.0 : -1.0;
        std::vector<Complex> out(n);

        for (std::size_t k = 0; k < n; ++k)
        {
            Complex sum(0.0, 0.0);
            for (std::size_t t = 0; t < n; ++t)
            {
                // Reduce k * t modulo n first so the angle stays small and exact.
                const std::size_t kt = (k * t) % n;
                sum += data[t] * std::polar(1.0, sign * 2.0 * std::numbers::pi * static_cast<double>(kt) / static_cast<double>(n));
            }

            out[k] = inverse ? sum / static_cast<double>(n) : sum;
        }

        return out;
    }
} // namespace SparkyStudios::Audio::Amplitude::Fidelity
