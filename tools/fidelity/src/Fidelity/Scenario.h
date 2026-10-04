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

#ifndef _AM_FIDELITY_SCENARIO_H
#define _AM_FIDELITY_SCENARIO_H

#include <cstdint>
#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <Fidelity/RenderSession.h>
#include <Fidelity/Signal.h>

namespace SparkyStudios::Audio::Amplitude::Fidelity
{
    /**
     * @brief Grid dimensions a scenario sweeps in the full grid.
     */
    enum GridDimension : std::uint32_t
    {
        kGridBlockSize = 1u << 0,
        kGridFps = 1u << 1,
        kGridJitter = 1u << 2,
        kGridOutputRate = 1u << 3,
    };

    enum class GridMode
    {
        /// Block 1024, 60 fps, no jitter, 48 kHz.
        Quick,
        /// Every value of the dimensions the scenario declares.
        Full,
    };

    /**
     * @brief One combination of render parameters.
     */
    struct GridPoint
    {
        std::uint32_t blockSize = 1024;
        double fps = 60.0;
        bool jitter = false;
        std::uint32_t outputRate = 48000;

        /**
         * @brief Stable identifier, e.g. "b1024_f60_j0_r48000".
         */
        [[nodiscard]] std::string Key() const;
    };

    [[nodiscard]] std::vector<GridPoint> ExpandGrid(GridMode mode, std::uint32_t dimensions);

    enum class Better
    {
        Lower,
        Higher,
    };

    /**
     * @brief A measured value. Metrics with a target are gated; the others are informative.
     */
    struct Metric
    {
        std::string name;
        double value = 0.0;
        std::string unit;
        Better better = Better::Lower;
        std::optional<double> target;
    };

    [[nodiscard]] bool MeetsTarget(double value, Better better, double target);

    /**
     * @brief False for a metric without a target or with a non-finite value.
     */
    [[nodiscard]] bool MeetsTarget(const Metric& metric);

    /**
     * @brief A localised finding (a click, a phase jump), tied to the timeline.
     */
    struct EventRecord
    {
        std::string analyzer;
        std::uint32_t channel = 0;
        std::uint64_t sample = 0;
        double levelDbfs = -400.0;
        double aboveFloorDb = 0.0;
        /// Label of the latest action at or before the event, and the distance from it in samples.
        std::string nearestAction;
        std::int64_t actionOffset = 0;
        /// Distance from the start of the audio block holding the event.
        std::int64_t blockOffset = 0;
    };

    /**
     * @brief The result of one scenario variant at one grid point.
     */
    struct Measurement
    {
        std::string variant;
        GridPoint point;
        std::vector<Metric> metrics;
        std::vector<EventRecord> events;
        Capture capture;
        /// Optional residual signal (null tests), one vector per channel.
        std::vector<std::vector<float>> residual;
        /// Non-empty when the render or the analysis failed.
        std::string error;

        void Add(std::string name, double value, std::string unit, Better better, std::optional<double> target = std::nullopt);
        [[nodiscard]] const Metric* Find(std::string_view name) const;
    };

    struct ScenarioResult
    {
        std::string id;
        std::string title;
        std::vector<Measurement> measurements;
    };

    struct RunContext
    {
        std::filesystem::path assets = kDefaultAssetsPath;
        GridMode grid = GridMode::Quick;
    };

    /**
     * @brief Builds a fresh action timeline; called once per render so no state leaks between renders.
     */
    using ActionFactory = std::function<std::vector<TimedAction>()>;

    /**
     * @brief A measurable situation: a stimulus, a timeline of public-API actions, and the analyses of the capture.
     */
    class Scenario
    {
    public:
        virtual ~Scenario() = default;

        /// Short identifier, e.g. "P1".
        [[nodiscard]] virtual std::string Id() const = 0;
        [[nodiscard]] virtual std::string Title() const = 0;
        /// GridDimension mask swept by the full grid.
        [[nodiscard]] virtual std::uint32_t Dimensions() const = 0;
        [[nodiscard]] virtual std::vector<std::string> Variants() const = 0;
        virtual void Measure(const RunContext& context, const GridPoint& point, const std::string& variant, Measurement& out) const = 0;
    };

    class ScenarioRegistry
    {
    public:
        void Add(std::unique_ptr<Scenario> scenario);
        [[nodiscard]] const std::vector<std::unique_ptr<Scenario>>& All() const;
        /// Scenarios whose id matches the glob ('*' and '?').
        [[nodiscard]] std::vector<const Scenario*> Match(std::string_view glob) const;

    private:
        std::vector<std::unique_ptr<Scenario>> _scenarios;
    };

    [[nodiscard]] bool GlobMatch(std::string_view pattern, std::string_view text);

    /**
     * @brief Runs every variant at every grid point. Exceptions become measurement errors.
     */
    [[nodiscard]] ScenarioResult RunScenario(const Scenario& scenario, const RunContext& context);

    /**
     * @brief Renders twice, records the bit-exact mismatch count, and returns the first capture in @p capture.
     */
    bool RenderDeterministic(
        const RunContext& context,
        const RenderSettings& settings,
        const ActionFactory& makeActions,
        std::string_view tag,
        Measurement& out,
        Capture& capture);

    /**
     * @brief Fills each event's nearest action and block offsets from the capture's markers.
     */
    void AnnotateEvents(std::vector<EventRecord>& events, const Capture& capture);

    [[nodiscard]] Signal ChannelSignal(const Capture& capture, std::size_t channel);

    /**
     * @brief Registers every scenario of the harness (defined with the scenarios).
     */
    void RegisterAllScenarios(ScenarioRegistry& registry);
} // namespace SparkyStudios::Audio::Amplitude::Fidelity

#endif // _AM_FIDELITY_SCENARIO_H
