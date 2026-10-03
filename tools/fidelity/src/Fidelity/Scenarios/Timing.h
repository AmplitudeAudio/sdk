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

#ifndef _AM_FIDELITY_SCENARIOS_TIMING_H
#define _AM_FIDELITY_SCENARIOS_TIMING_H

#include <cstdint>

#include <Fidelity/Scenario.h>
#include <Fidelity/Stimuli.h>

namespace SparkyStudios::Audio::Amplitude::Fidelity
{
    /**
     * @brief P2 timing on a capture of @p spec played at @p playAt and stopped at @p stopAt with a @p fadeMs fade.
     *
     * Adds start.latencyMs (onset past the asset's own fade-in), stop.latencyMs (when the modelled fade starts),
     * stop.fadeMs (10-90 %), stop.fadeCurveErrorDb (when fadeMs > 0) and stop.tailDbfs. Levels come from the measured
     * plateau and crossings from the smoothed envelope, so zeroed samples and off-nominal levels do not move them.
     */
    void MeasureStopTiming(
        const Capture& capture, const StimulusSpec& spec, std::uint64_t playAt, std::uint64_t stopAt, double fadeMs, Measurement& out);

    /**
     * @brief P3 pause/resume continuity on a chirp capture: resume.positionErrorSamples, pause.fadeMs, resume.fadeMs.
     * Returns false (with out.error set) when the edges cannot be found.
     */
    bool MeasurePauseResume(
        const Capture& capture, const StimulusSpec& spec, std::uint64_t pauseAt, std::uint64_t resumeAt, Measurement& out);

    /**
     * @brief P3 seek latency on a chirp capture: seek.latencyMs. Returns false (with out.error set) when the chirp cannot
     * be tracked after the seek.
     */
    bool MeasureSeek(const Capture& capture, const StimulusSpec& spec, std::uint64_t seekAt, double seekSeconds, Measurement& out);

    /**
     * @brief P7 on a capture of @p spec played at @p playAt: length.errorSamples, tail.peakDbfs, integrity metrics.
     * Returns false (with out.error set) when the start or the end cannot be found.
     */
    bool MeasureEndOfSound(
        const Capture& capture, const StimulusSpec& spec, std::uint64_t playAt, std::uint64_t outFrames, Measurement& out);
} // namespace SparkyStudios::Audio::Amplitude::Fidelity

#endif // _AM_FIDELITY_SCENARIOS_TIMING_H
