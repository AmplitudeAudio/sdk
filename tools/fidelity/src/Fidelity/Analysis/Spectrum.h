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

#ifndef _AM_FIDELITY_ANALYSIS_SPECTRUM_H
#define _AM_FIDELITY_ANALYSIS_SPECTRUM_H

#include <cstddef>
#include <span>
#include <vector>

namespace SparkyStudios::Audio::Amplitude::Fidelity
{
    /**
     * @brief One-sided averaged power spectrum. A full-scale sine's main lobe sums to 0.5.
     */
    struct PowerSpectrum
    {
        double sampleRate = 0.0;
        std::size_t frameSize = 0;
        double binHz = 0.0;
        std::vector<double> power;
    };

    /**
     * @brief Welch spectrum: 7-term Blackman-Harris frames with 50 % overlap, averaged.
     *
     * The frame shrinks to the largest power of two that fits when the signal is shorter than @p frameSize.
     */
    [[nodiscard]] PowerSpectrum AveragedPowerSpectrum(std::span<const double> x, double sampleRate, std::size_t frameSize);

    /**
     * @brief Sum of the bins within ±halfWidthBins of the bin nearest @p frequencyHz.
     */
    [[nodiscard]] double BandPower(const PowerSpectrum& spectrum, double frequencyHz, std::size_t halfWidthBins);

    /**
     * @brief Options for AnalyzeSpectrum().
     */
    struct SpectrumOptions
    {
        double fundamentalHz = 997.0;
        std::size_t frameSize = 32768;
        std::size_t mainLobeBins = 8;
        std::size_t harmonics = 10;
        double bandLowHz = 20.0;
        double bandHighHz = 20000.0;
    };

    /**
     * @brief Result of AnalyzeSpectrum(). Ratios are relative to the fundamental's power.
     */
    struct SpectrumResult
    {
        double fundamentalDbfs = -400.0;
        double thdDb = -400.0;
        double thdnDb = -400.0;
        double sinadDb = 400.0;
        double worstSpurDbc = -400.0;
        double worstSpurHz = 0.0;
        /// Median in-band bin outside the fundamental and harmonics, relative to a full-scale sine.
        double noiseFloorDbfs = -400.0;
    };

    /**
     * @brief Steady-state tone analysis: THD, THD+N, SINAD, worst non-harmonic spur and noise floor.
     */
    [[nodiscard]] SpectrumResult AnalyzeSpectrum(std::span<const double> x, double sampleRate, const SpectrumOptions& options);
} // namespace SparkyStudios::Audio::Amplitude::Fidelity

#endif // _AM_FIDELITY_ANALYSIS_SPECTRUM_H
