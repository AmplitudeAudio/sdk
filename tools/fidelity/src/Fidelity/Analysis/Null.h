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

#ifndef _AM_FIDELITY_ANALYSIS_NULL_H
#define _AM_FIDELITY_ANALYSIS_NULL_H

#include <cstddef>
#include <cstdint>
#include <span>

#include <Fidelity/Signal.h>

namespace SparkyStudios::Audio::Amplitude::Fidelity
{
    /**
     * @brief Options for AnalyzeNull().
     */
    struct NullOptions
    {
        /// Largest distance from lagCenter searched, in samples, in both directions.
        std::size_t maxLag = 4096;
        /// Centre of the lag search: a coarse lag from elsewhere (e.g. OnsetSample of both signals), refined here.
        std::int64_t lagCenter = 0;
        /// Least-squares gain match of b onto a before subtracting.
        bool matchGain = true;
        /// Samples excluded at both ends of the overlap.
        std::size_t guard = 0;
        /// Window length of the per-window residual peaks.
        std::size_t windowSize = 480;
    };

    /**
     * @brief Result of AnalyzeNull().
     */
    struct NullResult
    {
        /// a[n] is matched against b[n - lag].
        std::int64_t lag = 0;
        double gainDb = 0.0;
        double residualRmsDbfs = -400.0;
        double residualPeakDbfs = -400.0;
        /// Index into a of the window holding the largest residual.
        std::size_t worstWindowStart = 0;
        /// Index into a of residual[0].
        std::size_t residualBegin = 0;
        Signal residual;
    };

    /**
     * @brief Aligns @p b to @p a by cross-correlation, optionally gain-matches it, and measures a - g * b.
     */
    [[nodiscard]] NullResult AnalyzeNull(std::span<const double> a, std::span<const double> b, const NullOptions& options);

    /**
     * @brief Index of the first sample whose magnitude reaches @p threshold; x.size() when none does.
     */
    [[nodiscard]] std::size_t OnsetSample(std::span<const double> x, double threshold);

    /**
     * @brief Number of samples whose bit patterns differ; a length difference counts as that many differences.
     */
    [[nodiscard]] std::size_t CountDifferences(std::span<const float> a, std::span<const float> b);
} // namespace SparkyStudios::Audio::Amplitude::Fidelity

#endif // _AM_FIDELITY_ANALYSIS_NULL_H
