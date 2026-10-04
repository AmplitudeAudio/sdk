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

#ifndef _AM_FIDELITY_ANALYSIS_ENVELOPE_H
#define _AM_FIDELITY_ANALYSIS_ENVELOPE_H

#include <cstddef>
#include <span>
#include <vector>

#include <Fidelity/Signal.h>

namespace SparkyStudios::Audio::Amplitude::Fidelity
{
    /**
     * @brief Options for AnalyzeEnvelope().
     */
    struct EnvelopeOptions
    {
        double carrierHz = 997.0;
        /// Analysed region [begin, end); end 0 means the whole signal.
        std::size_t begin = 0;
        std::size_t end = 0;
        /// Expected linear envelope (same length as the signal). Empty: a moving average of the measured envelope.
        Signal expected;
        std::size_t smoothWindow = 4096;
        /// A step is the change of the envelope error across ±stepSpan samples (one 997 Hz period at 48 kHz, which keeps
        /// the analytic-signal ringing around a step to about 5 % of it).
        std::size_t stepSpan = 48;
        double stepThresholdDb = 0.01;
        /// Samples whose reference level is below this are ignored.
        double minLevelDbfs = -60.0;
        /// Update rates (block rate, frame rate) whose sidebands are measured.
        std::vector<double> updateRatesHz;
        std::size_t sidebandOrders = 8;
    };

    /**
     * @brief A modulation sideband at carrier ± order * rate.
     */
    struct Sideband
    {
        double rateHz = 0.0;
        std::size_t order = 0;
        double frequencyHz = 0.0;
        double levelDbc = -400.0;
    };

    /**
     * @brief Result of AnalyzeEnvelope().
     */
    struct EnvelopeResult
    {
        /// 20 log10 of the RMS of (envelope / reference - 1).
        double modulationNoiseDbc = -400.0;
        double maxErrorDb = 0.0;
        double rmsErrorDb = 0.0;
        std::size_t stepCount = 0;
        double largestStepDb = 0.0;
        std::size_t largestStepSample = 0;
        double worstSidebandDbc = -400.0;
        std::vector<Sideband> sidebands;
        /// Envelope of the whole signal.
        Signal envelope;
    };

    /**
     * @brief Envelope error against a reference, gain steps (zipper), and update-rate sidebands of a tone.
     */
    [[nodiscard]] EnvelopeResult AnalyzeEnvelope(std::span<const double> x, double sampleRate, const EnvelopeOptions& options);

    /**
     * @brief First time (fractional sample, linearly interpolated) after @p searchBegin where @p envelope crosses
     * @p level in the given direction; -1 when it never does.
     */
    [[nodiscard]] double CrossingTime(std::span<const double> envelope, double level, double searchBegin, bool rising);

    /**
     * @brief Moving-average length of TimingEnvelope() (about 2 ms at 48 kHz, odd so the average is centred): long enough to flatten a
     * single-sample dip and its analytic-signal ringing, short enough to keep a hard cut within a sample of its place.
     */
    constexpr std::size_t kTimingSmoothing = 97;

    /**
     * @brief How long the envelope must stay past a threshold for SustainedCrossing() to accept the crossing.
     */
    constexpr std::size_t kTimingHold = 96;

    /**
     * @brief Envelope of @p x smoothed for timing measurements (see kTimingSmoothing).
     */
    [[nodiscard]] Signal TimingEnvelope(std::span<const double> x);

    /**
     * @brief Median of the envelope over [begin, end): the level a capture actually plays at. Timing thresholds are
     * fractions of this, never of the nominal level.
     */
    [[nodiscard]] double PlateauLevel(std::span<const double> envelope, std::size_t begin, std::size_t end);

    /**
     * @brief Like CrossingTime(), but only accepts a crossing after which the envelope stays past @p level for
     * @p hold samples; -1 when there is none.
     */
    [[nodiscard]] double SustainedCrossing(
        std::span<const double> envelope, double level, double searchBegin, bool rising, std::size_t hold);

    /**
     * @brief The last time the envelope falls through @p level, searching backward from its end; -1 when it never does.
     */
    [[nodiscard]] double LastFallingCrossing(std::span<const double> envelope, double level);

    /**
     * @brief Timing of the 10 % to 90 % part of a move of the envelope from @p from to @p to.
     */
    struct FadeTiming
    {
        bool found = false;
        double start = 0.0;
        double end = 0.0;
        double durationMs = 0.0;
    };

    /**
     * @brief Measures a fade in @p envelope from level @p from to level @p to (linear amplitudes).
     */
    [[nodiscard]] FadeTiming MeasureFade(std::span<const double> envelope, double sampleRate, double from, double to, double searchBegin);
} // namespace SparkyStudios::Audio::Amplitude::Fidelity

#endif // _AM_FIDELITY_ANALYSIS_ENVELOPE_H
