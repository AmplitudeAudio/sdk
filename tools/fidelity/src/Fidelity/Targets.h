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

#ifndef _AM_FIDELITY_TARGETS_H
#define _AM_FIDELITY_TARGETS_H

/**
 * @brief Fidelity targets ("inaudible, pro-engine class"). Each is a one-line decision.
 */
namespace SparkyStudios::Audio::Amplitude::Fidelity::Targets
{
    constexpr double kMaxNanCount = 0.0;
    constexpr double kMaxInfCount = 0.0;
    constexpr double kMaxDenormalCount = 0.0;
    constexpr double kMaxClippedSamples = 0.0;
    constexpr double kMaxDropouts = 0.0;

    /// Worst click-band event on a -6 dBFS tone.
    constexpr double kMaxClickDbfs = -90.0;

    constexpr double kMaxThdnDb = -100.0;
    constexpr double kMaxSpurDbc = -100.0;
    constexpr double kMaxPassbandRippleDb = 0.1;

    constexpr double kMaxEnvelopeStepDb = 0.05;
    constexpr double kMaxSidebandDbc = -90.0;
    constexpr double kMaxRampErrorDb = 0.1;

    constexpr double kMaxPitchStepCents = 0.5;
    constexpr double kMaxPitchRmsDeviationCents = 1.0;
    constexpr double kMaxPhaseJumps = 0.0;

    constexpr double kMaxNullResidualDbfs = -120.0;
    constexpr double kMaxDeterminismMismatches = 0.0;

    /// Largest spatial trajectory step relative to the expected per-window change.
    constexpr double kMaxSpatialStepRatio = 1.5;

    constexpr double kMaxResumePositionErrorSamples = 1.0;
    constexpr double kMaxLengthErrorSamples = 2.0;
    constexpr double kMaxTailDbfs = -120.0;
} // namespace SparkyStudios::Audio::Amplitude::Fidelity::Targets

/**
 * @brief The finest value each analyzer resolves, as validated by the fidelity_analysis tests (float32 captures).
 */
namespace SparkyStudios::Audio::Amplitude::Fidelity::Floors
{
    constexpr double kClickDbfs = -120.0;
    constexpr double kSpectrumDb = -120.0;
    constexpr double kRippleDb = 0.01;
    constexpr double kEnvelopeStepDb = 0.005;
    constexpr double kSidebandDbc = -120.0;
    constexpr double kRampErrorDb = 0.01;
    constexpr double kPitchStepCents = 0.05;
    constexpr double kPitchRmsCents = 0.05;
    constexpr double kNullDbfs = -140.0;
    constexpr double kPositionSamples = 0.1;
    constexpr double kLengthSamples = 0.5;
} // namespace SparkyStudios::Audio::Amplitude::Fidelity::Floors

#endif // _AM_FIDELITY_TARGETS_H
