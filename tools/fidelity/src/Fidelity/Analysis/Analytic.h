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

#ifndef _AM_FIDELITY_ANALYSIS_ANALYTIC_H
#define _AM_FIDELITY_ANALYSIS_ANALYTIC_H

#include <span>
#include <vector>

#include <Fidelity/Analysis/Fft.h>
#include <Fidelity/Signal.h>

namespace SparkyStudios::Audio::Amplitude::Fidelity
{
    /**
     * @brief Samples at each end tapered before the Hilbert transform; results there are unreliable.
     *
     * Scenarios keep their first action at least this far into the capture.
     */
    constexpr std::size_t kAnalyticTaper = 4096;

    /**
     * @brief Analytic signal of @p x (x + i H{x}) via FFT.
     *
     * Both ends are tapered over min(kAnalyticTaper, n / 8) samples and the transform is zero-padded to twice the length,
     * so truncation edges do not leak into the rest of the signal.
     */
    [[nodiscard]] std::vector<Complex> AnalyticSignal(std::span<const double> x);

    /**
     * @brief |AnalyticSignal(x)|.
     */
    [[nodiscard]] Signal Envelope(std::span<const double> x);

    /**
     * @brief Unwrapped arg(AnalyticSignal(x)). For sin(theta) this is theta - pi / 2 (plus a multiple of 2 pi).
     */
    [[nodiscard]] Signal UnwrappedPhase(std::span<const double> x);

    /**
     * @brief Removes 2 pi jumps from a phase sequence in place.
     */
    void Unwrap(Signal& phase);
} // namespace SparkyStudios::Audio::Amplitude::Fidelity

#endif // _AM_FIDELITY_ANALYSIS_ANALYTIC_H
