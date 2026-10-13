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

#pragma once

#ifndef _AM_FIDELITY_ANALYSIS_FFT_H
#define _AM_FIDELITY_ANALYSIS_FFT_H

#include <complex>
#include <cstddef>
#include <vector>

namespace SparkyStudios::Audio::Amplitude::Fidelity
{
    using Complex = std::complex<double>;

    /**
     * @brief Returns whether @p n is a non-zero power of two.
     */
    [[nodiscard]] bool IsPowerOfTwo(std::size_t n);

    /**
     * @brief Returns the smallest power of two greater than or equal to @p n (1 for 0).
     */
    [[nodiscard]] std::size_t NextPowerOfTwo(std::size_t n);

    /**
     * @brief In-place radix-2 FFT. The size must be a power of two.
     *
     * The inverse transform is scaled by 1/N, so a forward then inverse transform returns the input.
     */
    void Fft(std::vector<Complex>& data, bool inverse);

    /**
     * @brief O(N^2) DFT with the same conventions as Fft(); the reference that validates it.
     */
    [[nodiscard]] std::vector<Complex> NaiveDft(const std::vector<Complex>& data, bool inverse);
} // namespace SparkyStudios::Audio::Amplitude::Fidelity

#endif // _AM_FIDELITY_ANALYSIS_FFT_H
