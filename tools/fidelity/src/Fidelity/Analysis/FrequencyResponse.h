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

#ifndef _AM_FIDELITY_ANALYSIS_FREQUENCY_RESPONSE_H
#define _AM_FIDELITY_ANALYSIS_FREQUENCY_RESPONSE_H

#include <cstddef>
#include <span>
#include <vector>

#include <Fidelity/Signal.h>

namespace SparkyStudios::Audio::Amplitude::Fidelity
{
    /**
     * @brief Exponential sine sweep from f1 to f2 with raised-cosine fades at both ends.
     */
    struct SweepModel
    {
        double f1Hz = 10.0;
        double f2Hz = 20000.0;
        double durationSeconds = 4.0;
        double amplitude = 0.5;
        double fadeSeconds = 0.01;
    };

    /**
     * @brief Renders the sweep at @p sampleRate. The part above @p maxFrequencyHz is faded out and zeroed, so a model
     * whose f2 exceeds the rate's Nyquist frequency can still serve as a reference.
     */
    [[nodiscard]] Signal RenderLogSweep(const SweepModel& model, double sampleRate, double maxFrequencyHz);

    /**
     * @brief Options for AnalyzeFrequencyResponse().
     */
    struct ResponseOptions
    {
        /// Length of the windowed impulse response (a power of two).
        std::size_t irLength = 16384;
        double passbandLowHz = 50.0;
        double passbandHighHz = 20000.0;
        /// The magnitude is normalised to 0 dB here.
        double referenceHz = 1000.0;
    };

    /**
     * @brief Result of AnalyzeFrequencyResponse().
     */
    struct ResponseResult
    {
        double rippleDb = 0.0;
        /// First frequency above referenceHz where the response drops below -3 dB; the last measured frequency if never.
        double minus3dBHz = 0.0;
        std::vector<double> frequencyHz;
        std::vector<double> magnitudeDb;
    };

    /**
     * @brief Measures the magnitude response of a captured sweep by Farina deconvolution, relative to the ideal sweep
     * rendered at the capture's rate. Only frequencies inside the sweep range can be observed.
     */
    [[nodiscard]] ResponseResult AnalyzeFrequencyResponse(
        std::span<const double> captured, double sampleRate, const SweepModel& model, const ResponseOptions& options);
} // namespace SparkyStudios::Audio::Amplitude::Fidelity

#endif // _AM_FIDELITY_ANALYSIS_FREQUENCY_RESPONSE_H
