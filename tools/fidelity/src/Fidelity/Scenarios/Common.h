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

#ifndef _AM_FIDELITY_SCENARIOS_COMMON_H
#define _AM_FIDELITY_SCENARIOS_COMMON_H

#include <cstdint>
#include <initializer_list>
#include <span>
#include <string>
#include <vector>

#include <Fidelity/Scenario.h>
#include <Fidelity/Stimuli.h>

namespace SparkyStudios::Audio::Amplitude::Fidelity
{
    /**
     * @brief Silence before the first action: keeps onsets clear of the analytic-signal taper.
     */
    constexpr std::uint64_t kLeadIn = 4800;

    [[nodiscard]] std::uint64_t Seconds(double seconds, double rate);

    /**
     * @brief Output frames a stimulus lasts at @p outputRate (rounded up).
     */
    [[nodiscard]] std::uint64_t OutputFrames(const StimulusSpec& spec, std::uint32_t outputRate);

    /**
     * @brief Left (= right) gain the stereo panner applies to a non-spatialized sound.
     */
    [[nodiscard]] double CenterPanGain();

    /**
     * @brief Settings for the isolated config matching @p point.
     */
    [[nodiscard]] RenderSettings IsolatedSettings(const GridPoint& point, std::uint64_t durationSamples);

    /**
     * @brief A timeline that plays @p soundName at kLeadIn (action "play").
     */
    [[nodiscard]] ActionFactory PlayOnly(const std::string& soundName);

    /**
     * @brief Sets out.error and returns false when channel 0 peaks under -60 dBFS in [begin, end).
     */
    bool RequireSignal(Measurement& out, const Capture& capture, std::uint64_t begin, std::uint64_t end);

    void AddIntegrityMetrics(Measurement& out, const Capture& capture, std::uint64_t signalBegin, std::uint64_t signalEnd);

    /**
     * @brief Click detection on every channel; events in [begin, end) count.
     */
    void AddClickMetrics(Measurement& out, const Capture& capture, double highPassHz, std::uint64_t begin, std::uint64_t end);

    [[nodiscard]] std::vector<std::string> StimulusNames(std::initializer_list<StimulusKind> kinds);

    /**
     * @brief Median of the finite values; NaN when there are none.
     */
    [[nodiscard]] double MedianFinite(std::vector<double> values);

    /**
     * @brief Time (fractional sample) where the straight line through a fade's 20 % and 80 % points of @p level reaches
     * zero, searching from @p searchBegin; -1 when the fade is not found.
     */
    [[nodiscard]] double FadeZero(std::span<const double> envelope, double level, double searchBegin, bool rising);
} // namespace SparkyStudios::Audio::Amplitude::Fidelity

#endif // _AM_FIDELITY_SCENARIOS_COMMON_H
