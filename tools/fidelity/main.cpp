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

#include <cstdio>
#include <ctime>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string>

#include <CLI/CLI.hpp>

#include <Fidelity/AssetGenerator.h>
#include <Fidelity/RenderSession.h>
#include <Fidelity/Report.h>
#include <Fidelity/Scenario.h>

using namespace SparkyStudios::Audio::Amplitude::Fidelity;

namespace
{
    std::string Timestamp()
    {
        const std::time_t now = std::time(nullptr);
        std::tm local{};
#if defined(_WIN32)
        localtime_s(&local, &now);
#else
        localtime_r(&now, &local);
#endif
        char buffer[32];
        std::strftime(buffer, sizeof(buffer), "%Y%m%d-%H%M%S", &local);
        return buffer;
    }

    std::string GitSha()
    {
#if defined(_WIN32)
        FILE* pipe = _popen("git rev-parse --short HEAD 2>NUL", "r");
#else
        FILE* pipe = popen("git rev-parse --short HEAD 2>/dev/null", "r");
#endif
        if (pipe == nullptr)
            return "unknown";

        char buffer[64] = {};
        const bool read = std::fgets(buffer, sizeof(buffer), pipe) != nullptr;
#if defined(_WIN32)
        _pclose(pipe);
#else
        pclose(pipe);
#endif
        std::string sha = read ? buffer : "unknown";
        while (!sha.empty() && (sha.back() == '\n' || sha.back() == '\r'))
            sha.pop_back();

        return sha.empty() ? "unknown" : sha;
    }

    const char* BuildMode()
    {
#if defined(NDEBUG)
        return "release";
#else
        return "debug";
#endif
    }
} // namespace

int main(int argc, char** argv)
{
    CLI::App app{ "Amplitude fidelity harness: renders engine scenarios offline and measures their audio quality." };

    bool list = false;
    bool noWav = false;
    bool generate = false;
    std::string filter = "*";
    std::string grid = "quick";
    std::string out;
    std::string baselinePath;
    std::string writeBaseline;
    std::string assets = kDefaultAssetsPath;
    std::string project;
    std::string resampler;

    app.add_flag("--list", list, "List scenarios and exit.");
    app.add_option("--filter", filter, "Glob on scenario ids, e.g. \"P*\".");
    app.add_option("--grid", grid, "Parameter grid.")->check(CLI::IsMember({ "quick", "full" }));
    app.add_option("--out", out, "Output directory (default: fidelity/<timestamp>).");
    app.add_option("--baseline", baselinePath, "metrics.tsv of a previous run to compare against.");
    app.add_flag("--no-wav", noWav, "Do not write WAV captures.");
    app.add_option("--write-baseline", writeBaseline, "Also write the metrics to this baseline file (quick grid only).");
    app.add_option("--assets", assets, "Fidelity asset directory.");
    app.add_flag("--generate-assets", generate, "Generate the fidelity project files and stimuli, then exit.");
    app.add_option("--project", project, "Project directory written by --generate-assets.");
    app.add_option("--resampler", resampler, "Render with this registered resampler instead of the config's (default: the config's).");

    CLI11_PARSE(app, argc, argv);

    if (generate)
    {
        if (project.empty())
        {
            std::cerr << "--generate-assets needs --project.\n";
            return 1;
        }

        return GenerateAssets({ project, assets }) ? 0 : 1;
    }

    ScenarioRegistry registry;
    RegisterAllScenarios(registry);

    if (list)
    {
        for (const auto& scenario : registry.All())
            std::cout << scenario->Id() << "  " << scenario->Title() << "  (" << scenario->Variants().size() << " variants)\n";

        return 0;
    }

    const GridMode mode = grid == "full" ? GridMode::Full : GridMode::Quick;
    if (!writeBaseline.empty() && mode != GridMode::Quick)
    {
        std::cerr << "--write-baseline requires --grid quick.\n";
        return 1;
    }

    if (!writeBaseline.empty() && filter != "*")
    {
        std::cerr << "--write-baseline requires every scenario (no --filter): a partial baseline un-gates the rest.\n";
        return 1;
    }

    if (!writeBaseline.empty() && !resampler.empty() && resampler != "default")
    {
        std::cerr << "--write-baseline records the default resampler only.\n";
        return 1;
    }

    const std::vector<const Scenario*> selected = registry.Match(filter);
    if (selected.empty())
    {
        std::cerr << "No scenario matches '" << filter << "'.\n";
        return 1;
    }

    std::optional<Baseline> baseline;
    if (!baselinePath.empty())
    {
        baseline = ReadTsv(baselinePath);
        if (!baseline)
        {
            std::cerr << "Cannot read the baseline '" << baselinePath << "'.\n";
            return 1;
        }
    }

    RunContext context;
    context.assets = assets;
    context.grid = mode;
    context.resampler = resampler;

    std::vector<ScenarioResult> results;
    bool measurementErrors = false;
    for (const Scenario* scenario : selected)
    {
        std::cout << "[" << scenario->Id() << "] " << scenario->Title() << std::endl;
        results.push_back(RunScenario(*scenario, context));

        for (const Measurement& m : results.back().measurements)
        {
            if (m.error.empty())
                continue;

            measurementErrors = true;
            std::cerr << "  " << m.variant << " " << m.point.Key() << ": " << m.error << "\n";
        }
    }

    ReportOptions options;
    options.outDir = out.empty() ? std::filesystem::path("fidelity") / Timestamp() : std::filesystem::path(out);
    options.writeWavs = !noWav;
    options.gitSha = GitSha();
    options.buildMode = BuildMode();
    options.resampler = resampler.empty() ? "config" : resampler;

    if (!WriteReport(results, options, baseline ? &*baseline : nullptr))
    {
        std::cerr << "Cannot write the report to " << options.outDir.string() << ".\n";
        return 1;
    }

    if (!writeBaseline.empty() && measurementErrors)
    {
        std::cerr << "Not writing the baseline: some measurements failed (listed above).\n";
        return 2;
    }

    if (!writeBaseline.empty() && !WriteTsv(writeBaseline, results))
    {
        std::cerr << "Cannot write the baseline to " << writeBaseline << ".\n";
        return 1;
    }

    std::size_t gated = 0;
    std::size_t meets = 0;
    for (const ScenarioResult& result : results)
        for (const Measurement& m : result.measurements)
            for (const Metric& metric : m.metrics)
                if (metric.target)
                {
                    ++gated;
                    meets += MeetsTarget(metric) ? 1 : 0;
                }

    std::cout << "Report: " << options.outDir.string() << "\n" << meets << " of " << gated << " gated metrics meet their target.\n";

    if (baseline)
    {
        std::size_t regressed = 0;
        for (const Comparison& comparison : CompareToBaseline(results, *baseline))
        {
            if (comparison.verdict != Verdict::Regressed && comparison.verdict != Verdict::Missing)
                continue;

            ++regressed;
            std::cout << "  " << VerdictName(comparison.verdict) << ": " << comparison.key << " (" << comparison.current << " vs "
                      << comparison.baseline << ")\n";
        }

        std::cout << regressed << " metrics regressed against the baseline.\n";
    }

    return measurementErrors ? 2 : 0;
}
