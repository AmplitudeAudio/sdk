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

#ifndef _AM_FIDELITY_ANALYSIS_INTEGRITY_H
#define _AM_FIDELITY_ANALYSIS_INTEGRITY_H

#include <cstddef>
#include <span>
#include <vector>

namespace SparkyStudios::Audio::Amplitude::Fidelity
{
    /**
     * @brief Options for AnalyzeIntegrity().
     */
    struct IntegrityOptions
    {
        /// First sample of the region where signal is expected (searched for dropouts).
        std::size_t signalBegin = 0;
        /// One past the last sample of that region; equal to signalBegin disables the dropout search.
        std::size_t signalEnd = 0;
        /// Shortest run of exact zeros counted as a dropout.
        std::size_t dropoutMinRun = 64;
        /// Samples whose magnitude reaches this value count as clipped.
        float clipThreshold = 1.0f;
    };

    /**
     * @brief Result of AnalyzeIntegrity().
     */
    struct IntegrityResult
    {
        std::size_t nanCount = 0;
        std::size_t infCount = 0;
        std::size_t denormalCount = 0;
        std::size_t clippedCount = 0;
        /// Mean over the signal region (the whole capture without one), in dBFS.
        double dcDbfs = -400.0;
        std::size_t dropoutCount = 0;
        std::size_t longestDropout = 0;
        std::vector<std::size_t> dropoutStarts;
    };

    /**
     * @brief Scans a float capture for NaN, infinities, denormals, clipping, DC offset and dropouts.
     */
    [[nodiscard]] IntegrityResult AnalyzeIntegrity(std::span<const float> x, const IntegrityOptions& options);
} // namespace SparkyStudios::Audio::Amplitude::Fidelity

#endif // _AM_FIDELITY_ANALYSIS_INTEGRITY_H
