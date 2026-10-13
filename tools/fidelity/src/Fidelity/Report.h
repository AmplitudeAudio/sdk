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

#ifndef _AM_FIDELITY_REPORT_H
#define _AM_FIDELITY_REPORT_H

#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <Fidelity/Scenario.h>

namespace SparkyStudios::Audio::Amplitude::Fidelity
{
    struct ReportOptions
    {
        std::filesystem::path outDir;
        bool writeWavs = true;
        std::string gitSha = "unknown";
        std::string buildMode = "unknown";
        std::string resampler = "config";
    };

    /**
     * @brief One line of a metrics TSV (a baseline or a previous run).
     */
    struct BaselineEntry
    {
        double value = 0.0;
        std::string unit;
        Better better = Better::Lower;
        std::optional<double> target;
    };

    using Baseline = std::map<std::string, BaselineEntry>;

    enum class Verdict
    {
        MeetsTarget,
        NoWorse,
        Improved,
        Regressed,
        New,
        Missing,
    };

    struct Comparison
    {
        std::string key;
        Verdict verdict = Verdict::New;
        double current = 0.0;
        double baseline = 0.0;
    };

    [[nodiscard]] const char* VerdictName(Verdict verdict);

    /**
     * @brief The preset a metrics TSV records for a run made with @p selected.
     *
     * "config", "default" and no selection at all render the same way, so they record the same name.
     *
     * @param[in] selected The --resampler selection, empty when the option was not passed.
     *
     * @return @p selected, or "default" when the run rendered through the default resampler.
     */
    [[nodiscard]] std::string_view ResamplerPreset(std::string_view selected);

    /**
     * @brief "<scenario>|<variant>|<grid key>|<metric>".
     */
    [[nodiscard]] std::string MetricKey(const std::string& scenario, const Measurement& measurement, const Metric& metric);

    [[nodiscard]] std::string JsonEscape(std::string_view text);

    /**
     * @brief Allowed regression per unit: dB/dBc/dBFS 0.5, cents 0.1, count 0, samples 0.5, ms 0.5, Hz 1.0, else 0.
     */
    [[nodiscard]] double ToleranceFor(std::string_view unit);

    /**
     * @brief Every metric as "key \t value \t unit \t better \t target" lines ("nan" and "-" for missing values).
     *
     * The header names the preset the numbers were rendered with, so a file written by another preset can be refused
     * instead of read as a regression.
     *
     * @param[in] results The run to serialise.
     * @param[in] resampler The preset the run rendered with, as returned by ResamplerPreset().
     */
    [[nodiscard]] std::string ToTsv(const std::vector<ScenarioResult>& results, std::string_view resampler = "default");
    bool WriteTsv(const std::filesystem::path& path, const std::vector<ScenarioResult>& results, std::string_view resampler = "default");

    /**
     * @brief Reads a metrics TSV written by ToTsv().
     *
     * @param[in] path The file to read.
     * @param[out] resampler When not null, receives the preset named in the header. A file that names none predates
     *                       the header and is read as the default preset, the only one --write-baseline accepts.
     *
     * @return The metrics, or nullopt when the file is unreadable or damaged.
     */
    [[nodiscard]] std::optional<Baseline> ReadTsv(const std::filesystem::path& path, std::string* resampler = nullptr);

    /**
     * @brief Compares the gated metrics (those with a target) with a baseline.
     */
    [[nodiscard]] std::vector<Comparison> CompareToBaseline(const std::vector<ScenarioResult>& results, const Baseline& baseline);

    /**
     * @brief Writes report.json, metrics.tsv, summary.md and (optionally) captures/ under options.outDir.
     */
    bool WriteReport(const std::vector<ScenarioResult>& results, const ReportOptions& options, const Baseline* baseline);
} // namespace SparkyStudios::Audio::Amplitude::Fidelity

#endif // _AM_FIDELITY_REPORT_H
