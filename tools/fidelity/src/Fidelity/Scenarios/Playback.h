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

#ifndef _AM_FIDELITY_SCENARIOS_PLAYBACK_H
#define _AM_FIDELITY_SCENARIOS_PLAYBACK_H

#include <memory>

#include <Fidelity/Scenario.h>

namespace SparkyStudios::Audio::Amplitude::Fidelity
{
    /**
     * @brief P1: resampling quality (THD+N, spurs, aliasing, passband).
     */
    [[nodiscard]] std::unique_ptr<Scenario> MakeResamplingQualityScenario();

    /**
     * @brief P2: start and stop (clicks, stop fade shape, silence after the stop).
     */
    [[nodiscard]] std::unique_ptr<Scenario> MakeStartStopScenario();

    /**
     * @brief P3: pause, resume and seek (clicks, source position continuity, seek latency).
     */
    [[nodiscard]] std::unique_ptr<Scenario> MakeTransportScenario();

    /**
     * @brief P4: loop seam of a seamless looping tone.
     */
    [[nodiscard]] std::unique_ptr<Scenario> MakeLoopSeamScenario();

    /**
     * @brief P7: end of sound (length, tail).
     */
    [[nodiscard]] std::unique_ptr<Scenario> MakeEndOfSoundScenario();

    /**
     * @brief Registers the playback-core scenarios (P1-P8).
     */
    void RegisterPlaybackScenarios(ScenarioRegistry& registry);
} // namespace SparkyStudios::Audio::Amplitude::Fidelity

#endif // _AM_FIDELITY_SCENARIOS_PLAYBACK_H
