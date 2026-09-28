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

#ifndef _AM_FIDELITY_ANALYSIS_CLICK_H
#define _AM_FIDELITY_ANALYSIS_CLICK_H

#include <cstddef>
#include <span>
#include <vector>

#include <Fidelity/Signal.h>

namespace SparkyStudios::Audio::Amplitude::Fidelity
{
    /**
     * @brief Options for AnalyzeClicks(). Only valid for stimuli with no content in the detection band.
     */
    struct ClickOptions
    {
        /// Lower edge of the detection band; keep it above the stimulus content (4 kHz or 2x the tone).
        double highPassHz = 4000.0;
        /// Raised-cosine transition width below highPassHz.
        double transitionHz = 1000.0;
        /// Samples ignored at both ends (capture truncation edges).
        std::size_t guard = 2048;
        /// Events quieter than this are ignored.
        double minEventDbfs = -120.0;
        /// Events must exceed the local median magnitude by this much.
        double marginDb = 12.0;
        /// Length of the window the local median is taken over.
        std::size_t floorWindow = 4096;
        /// A candidate must be the largest magnitude within this distance.
        std::size_t minSeparation = 64;
        /// Events closer than this merge into the loudest one (filter ringing).
        std::size_t mergeWindow = 256;
    };

    /**
     * @brief A detected click.
     */
    struct ClickEvent
    {
        std::size_t sample = 0;
        double peakDbfs = -400.0;
        double aboveFloorDb = 0.0;
    };

    /**
     * @brief Result of AnalyzeClicks().
     */
    struct ClickResult
    {
        std::vector<ClickEvent> events;
        /// Largest band magnitude over the analysed region, event or not.
        double maxPeakDbfs = -400.0;
        /// Median band magnitude over the analysed region.
        double floorDbfs = -400.0;
        /// The high-passed signal.
        Signal band;
    };

    /**
     * @brief Zero-phase FFT high-pass with a raised-cosine transition from (cutoff - transition) to cutoff.
     */
    [[nodiscard]] Signal HighPass(std::span<const double> x, double sampleRate, double cutoffHz, double transitionHz);

    /**
     * @brief Finds discontinuities as broadband energy above the stimulus band.
     */
    [[nodiscard]] ClickResult AnalyzeClicks(std::span<const double> x, double sampleRate, const ClickOptions& options);
} // namespace SparkyStudios::Audio::Amplitude::Fidelity

#endif // _AM_FIDELITY_ANALYSIS_CLICK_H
