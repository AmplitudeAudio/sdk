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

#ifndef _AM_FIDELITY_ANALYSIS_SPATIAL_H
#define _AM_FIDELITY_ANALYSIS_SPATIAL_H

#include <cstddef>
#include <span>
#include <vector>

namespace SparkyStudios::Audio::Amplitude::Fidelity
{
    /**
     * @brief Options for AnalyzeSpatial().
     */
    struct SpatialOptions
    {
        /// Window length (10 ms at 48 kHz).
        std::size_t window = 480;
        /// Largest interaural delay searched.
        double maxItdSeconds = 0.001;
        /// Analysed region [begin, end); end 0 means the whole signal.
        std::size_t begin = 0;
        std::size_t end = 0;
    };

    /**
     * @brief Per-window interaural level and time differences, and their largest window-to-window steps.
     */
    struct SpatialResult
    {
        std::vector<double> ildDb;
        /// Positive when the right channel lags the left. NaN for silent windows.
        std::vector<double> itdSamples;
        double largestIldStepDb = 0.0;
        std::size_t largestIldStepWindow = 0;
        double largestItdStepSamples = 0.0;
        std::size_t largestItdStepWindow = 0;
    };

    /**
     * @brief Tracks ILD and ITD over time for a two-channel capture.
     */
    [[nodiscard]] SpatialResult AnalyzeSpatial(
        std::span<const double> left, std::span<const double> right, double sampleRate, const SpatialOptions& options);
} // namespace SparkyStudios::Audio::Amplitude::Fidelity

#endif // _AM_FIDELITY_ANALYSIS_SPATIAL_H
