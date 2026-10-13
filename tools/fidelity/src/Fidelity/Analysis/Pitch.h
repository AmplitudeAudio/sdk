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

#ifndef _AM_FIDELITY_ANALYSIS_PITCH_H
#define _AM_FIDELITY_ANALYSIS_PITCH_H

#include <cstddef>
#include <span>
#include <vector>

#include <Fidelity/Signal.h>

namespace SparkyStudios::Audio::Amplitude::Fidelity
{
    /**
     * @brief Options for AnalyzePitch().
     */
    struct PitchOptions
    {
        /// Analysed region [begin, end); end 0 means the whole signal.
        std::size_t begin = 0;
        std::size_t end = 0;
        /// Expected instantaneous frequency per sample (same length as the signal); empty uses constantExpectedHz.
        Signal expectedHz;
        double constantExpectedHz = 0.0;
        /// Moving-average length applied to the instantaneous frequency (1 ms at 48 kHz).
        std::size_t smoothing = 48;
        /// |second difference| of the phase error above this is a phase jump.
        double phaseJumpRadians = 1e-3;
        /// A plateau is a run whose smoothed frequency stays within this many cents.
        double plateauToleranceCents = 0.1;
        std::size_t minPlateau = 256;
    };

    /**
     * @brief Result of AnalyzePitch().
     */
    struct PitchResult
    {
        double rmsDeviationCents = 0.0;
        double maxDeviationCents = 0.0;
        double largestStepCents = 0.0;
        std::size_t largestStepSample = 0;
        /// Median distance between consecutive frequency plateaus; 0 when the frequency moves continuously.
        double quantizationGridCents = 0.0;
        std::size_t phaseJumpCount = 0;
        std::vector<std::size_t> phaseJumps;
        /// Smoothed instantaneous frequency over the whole signal.
        Signal frequencyHz;
    };

    /**
     * @brief Tracks the instantaneous frequency of a tone and compares it with the expected curve.
     */
    [[nodiscard]] PitchResult AnalyzePitch(std::span<const double> x, double sampleRate, const PitchOptions& options);

    /**
     * @brief Linear chirp: x(tau) = amplitude * sin(2 pi ChirpCycles(tau)), tau in source seconds.
     */
    struct ChirpModel
    {
        double f0Hz = 200.0;
        double rateHzPerSecond = 450.0;
        double amplitude = 0.5;
    };

    /**
     * @brief Phase of the chirp in cycles at source time @p seconds: f0 tau + rate tau^2 / 2.
     */
    [[nodiscard]] double ChirpCycles(const ChirpModel& model, double seconds);

    /**
     * @brief Source time (seconds) of a captured chirp at every output sample.
     *
     * The instantaneous frequency gives a coarse time; the measured phase refines it to a fraction of a sample.
     * Samples whose envelope is below @p minLevel are NaN.
     */
    [[nodiscard]] Signal EstimateChirpSourceTime(std::span<const double> x, double outputRate, const ChirpModel& model, double minLevel);
} // namespace SparkyStudios::Audio::Amplitude::Fidelity

#endif // _AM_FIDELITY_ANALYSIS_PITCH_H
