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

#include <Fidelity/Scenario.h>

#include <cmath>
#include <cstdio>
#include <exception>

#include <Fidelity/Analysis/Null.h>
#include <Fidelity/Targets.h>

namespace SparkyStudios::Audio::Amplitude::Fidelity
{
    std::string GridPoint::Key() const
    {
        char buffer[96];
        std::snprintf(buffer, sizeof(buffer), "b%u_f%g_j%d_r%u", blockSize, fps, jitter ? 1 : 0, outputRate);
        return buffer;
    }

    std::vector<GridPoint> ExpandGrid(GridMode mode, std::uint32_t dimensions)
    {
        if (mode == GridMode::Quick)
            return { GridPoint{} };

        const std::vector<std::uint32_t> blocks =
            (dimensions & kGridBlockSize) != 0 ? std::vector<std::uint32_t>{ 256, 1024, 4096 } : std::vector<std::uint32_t>{ 1024 };
        const std::vector<double> rates =
            (dimensions & kGridFps) != 0 ? std::vector<double>{ 30.0, 60.0, 144.0 } : std::vector<double>{ 60.0 };
        const std::vector<bool> jitters = (dimensions & kGridJitter) != 0 ? std::vector<bool>{ false, true } : std::vector<bool>{ false };
        const std::vector<std::uint32_t> outputs =
            (dimensions & kGridOutputRate) != 0 ? std::vector<std::uint32_t>{ 48000, 44100 } : std::vector<std::uint32_t>{ 48000 };

        std::vector<GridPoint> points;
        for (const std::uint32_t output : outputs)
            for (const std::uint32_t block : blocks)
                for (const double fps : rates)
                    for (const bool jitter : jitters)
                        points.push_back({ block, fps, jitter, output });

        return points;
    }

    bool MeetsTarget(double value, Better better, double target)
    {
        if (!std::isfinite(value))
            return false;

        return better == Better::Lower ? value <= target : value >= target;
    }

    bool MeetsTarget(const Metric& metric)
    {
        return metric.target.has_value() && MeetsTarget(metric.value, metric.better, *metric.target);
    }

    void Measurement::Add(std::string name, double value, std::string unit, Better better, std::optional<double> target)
    {
        metrics.push_back({ std::move(name), value, std::move(unit), better, target });
    }

    const Metric* Measurement::Find(std::string_view name) const
    {
        for (const Metric& metric : metrics)
            if (metric.name == name)
                return &metric;

        return nullptr;
    }

    void ScenarioRegistry::Add(std::unique_ptr<Scenario> scenario)
    {
        _scenarios.push_back(std::move(scenario));
    }

    const std::vector<std::unique_ptr<Scenario>>& ScenarioRegistry::All() const
    {
        return _scenarios;
    }

    std::vector<const Scenario*> ScenarioRegistry::Match(std::string_view glob) const
    {
        std::vector<const Scenario*> matches;
        for (const auto& scenario : _scenarios)
            if (GlobMatch(glob, scenario->Id()))
                matches.push_back(scenario.get());

        return matches;
    }

    bool GlobMatch(std::string_view pattern, std::string_view text)
    {
        std::size_t p = 0;
        std::size_t t = 0;
        std::size_t star = std::string_view::npos;
        std::size_t mark = 0;

        while (t < text.size())
        {
            if (p < pattern.size() && (pattern[p] == '?' || pattern[p] == text[t]))
            {
                ++p;
                ++t;
            }
            else if (p < pattern.size() && pattern[p] == '*')
            {
                star = p++;
                mark = t;
            }
            else if (star != std::string_view::npos)
            {
                p = star + 1;
                t = ++mark;
            }
            else
            {
                return false;
            }
        }

        while (p < pattern.size() && pattern[p] == '*')
            ++p;

        return p == pattern.size();
    }

    ScenarioResult RunScenario(const Scenario& scenario, const RunContext& context)
    {
        ScenarioResult result{ scenario.Id(), scenario.Title(), {} };
        for (const GridPoint& point : ExpandGrid(context.grid, scenario.Dimensions()))
        {
            for (const std::string& variant : scenario.Variants())
            {
                Measurement measurement;
                measurement.variant = variant;
                measurement.point = point;

                try
                {
                    scenario.Measure(context, point, variant, measurement);
                } catch (const std::exception& exception)
                {
                    measurement.error = std::string("exception: ") + exception.what();
                } catch (...)
                {
                    measurement.error = "unknown exception";
                }

                result.measurements.push_back(std::move(measurement));
            }
        }

        return result;
    }

    bool RenderDeterministic(
        const RunContext& context,
        const RenderSettings& settings,
        const ActionFactory& makeActions,
        std::string_view tag,
        Measurement& out,
        Capture& capture)
    {
        RenderSettings local = settings;
        local.resampler = context.resampler;

        RenderOutcome first = Render(context.assets, local, makeActions());
        if (!first.error.empty())
        {
            out.error = first.error;
            return false;
        }

        const RenderOutcome second = Render(context.assets, local, makeActions());
        if (!second.error.empty())
        {
            out.error = second.error;
            return false;
        }

        std::size_t mismatches = 0;
        for (std::size_t c = 0; c < first.capture.channels.size(); ++c)
            mismatches += CountDifferences(first.capture.channels[c], second.capture.channels[c]);

        const std::string name = tag.empty() ? "determinism.mismatches" : "determinism." + std::string(tag) + ".mismatches";
        out.Add(name, static_cast<double>(mismatches), "count", Better::Lower, Targets::kMaxDeterminismMismatches);

        capture = std::move(first.capture);
        return true;
    }

    void AnnotateEvents(std::vector<EventRecord>& events, const Capture& capture)
    {
        for (EventRecord& event : events)
        {
            const Marker* action = nullptr;
            const Marker* block = nullptr;
            for (const Marker& marker : capture.markers)
            {
                if (marker.sample > event.sample)
                    break;

                if (marker.kind == MarkerKind::Action)
                    action = &marker;
                else if (marker.kind == MarkerKind::Block)
                    block = &marker;
            }

            if (action != nullptr)
            {
                event.nearestAction = action->label;
                event.actionOffset = static_cast<std::int64_t>(event.sample) - static_cast<std::int64_t>(action->sample);
            }

            if (block != nullptr)
                event.blockOffset = static_cast<std::int64_t>(event.sample) - static_cast<std::int64_t>(block->sample);
        }
    }

    Signal ChannelSignal(const Capture& capture, std::size_t channel)
    {
        return channel < capture.channels.size() ? ToSignal(capture.channels[channel]) : Signal{};
    }
} // namespace SparkyStudios::Audio::Amplitude::Fidelity
