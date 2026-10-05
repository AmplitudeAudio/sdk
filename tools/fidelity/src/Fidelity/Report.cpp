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

#include <Fidelity/Report.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <limits>
#include <set>
#include <sstream>

#include <Fidelity/Wav.h>

namespace SparkyStudios::Audio::Amplitude::Fidelity
{
    namespace
    {
        std::string Number(double value)
        {
            if (!std::isfinite(value))
                return "nan";

            char buffer[64];
            std::snprintf(buffer, sizeof(buffer), "%.6f", value);
            return buffer;
        }

        std::string JsonNumber(double value)
        {
            return std::isfinite(value) ? Number(value) : "null";
        }

        const char* BetterName(Better better)
        {
            return better == Better::Lower ? "lower" : "higher";
        }

        std::vector<std::string> Split(const std::string& line, char separator)
        {
            std::vector<std::string> fields;
            std::string field;
            std::istringstream stream(line);
            while (std::getline(stream, field, separator))
                fields.push_back(field);

            return fields;
        }

        bool WriteText(const std::filesystem::path& path, const std::string& text)
        {
            std::ofstream file(path, std::ios::binary | std::ios::trunc);
            file << text;
            return file.good();
        }

        std::vector<float> Interleave(const std::vector<std::vector<float>>& channels)
        {
            if (channels.empty())
                return {};

            std::size_t frames = channels[0].size();
            for (const auto& channel : channels)
                frames = std::min(frames, channel.size());

            std::vector<float> out(frames * channels.size());
            for (std::size_t i = 0; i < frames; ++i)
                for (std::size_t c = 0; c < channels.size(); ++c)
                    out[i * channels.size() + c] = channels[c][i];

            return out;
        }

        Verdict Judge(const Metric& metric, const BaselineEntry& entry)
        {
            if (!std::isfinite(metric.value))
                return Verdict::Regressed;

            const double target = *metric.target;
            if (std::isfinite(entry.value) && MeetsTarget(entry.value, metric.better, target))
                return MeetsTarget(metric.value, metric.better, target) ? Verdict::MeetsTarget : Verdict::Regressed;

            if (!std::isfinite(entry.value))
                return Verdict::Improved;

            const double tolerance = ToleranceFor(metric.unit);
            const bool lower = metric.better == Better::Lower;
            const bool worse = lower ? metric.value > entry.value + tolerance : metric.value < entry.value - tolerance;
            if (worse)
                return Verdict::Regressed;

            const bool better = lower ? metric.value < entry.value - tolerance : metric.value > entry.value + tolerance;
            return better ? Verdict::Improved : Verdict::NoWorse;
        }

        std::string TargetText(const Metric& metric)
        {
            if (!metric.target)
                return "—";

            return std::string(metric.better == Better::Lower ? "≤ " : "≥ ") + Number(*metric.target) + " " + metric.unit;
        }
    } // namespace

    const char* VerdictName(Verdict verdict)
    {
        switch (verdict)
        {
        case Verdict::MeetsTarget:
            return "meets target";
        case Verdict::NoWorse:
            return "no worse";
        case Verdict::Improved:
            return "improved";
        case Verdict::Regressed:
            return "REGRESSED";
        case Verdict::New:
            return "new";
        case Verdict::Missing:
            return "MISSING";
        }

        return "unknown";
    }

    std::string MetricKey(const std::string& scenario, const Measurement& measurement, const Metric& metric)
    {
        return scenario + "|" + measurement.variant + "|" + measurement.point.Key() + "|" + metric.name;
    }

    std::string JsonEscape(std::string_view text)
    {
        std::string out;
        for (const char c : text)
        {
            switch (c)
            {
            case '"':
                out += "\\\"";
                break;
            case '\\':
                out += "\\\\";
                break;
            case '\n':
                out += "\\n";
                break;
            case '\r':
                out += "\\r";
                break;
            case '\t':
                out += "\\t";
                break;
            default:
                if (static_cast<unsigned char>(c) < 0x20)
                {
                    char buffer[8];
                    std::snprintf(buffer, sizeof(buffer), "\\u%04x", static_cast<unsigned>(static_cast<unsigned char>(c)));
                    out += buffer;
                }
                else
                {
                    out += c;
                }
            }
        }

        return out;
    }

    double ToleranceFor(std::string_view unit)
    {
        if (unit == "dB" || unit == "dBc" || unit == "dBFS")
            return 0.5;
        if (unit == "cents")
            return 0.1;
        if (unit == "samples" || unit == "ms")
            return 0.5;
        if (unit == "Hz")
            return 1.0;

        return 0.0;
    }

    std::string ToTsv(const std::vector<ScenarioResult>& results)
    {
        std::ostringstream tsv;
        tsv << "# key\tvalue\tunit\tbetter\ttarget\n";
        for (const ScenarioResult& result : results)
            for (const Measurement& measurement : result.measurements)
                for (const Metric& metric : measurement.metrics)
                    tsv << MetricKey(result.id, measurement, metric) << '\t' << Number(metric.value) << '\t' << metric.unit << '\t'
                        << BetterName(metric.better) << '\t' << (metric.target ? Number(*metric.target) : "-") << '\n';

        return tsv.str();
    }

    bool WriteTsv(const std::filesystem::path& path, const std::vector<ScenarioResult>& results)
    {
        std::error_code error;
        if (path.has_parent_path())
            std::filesystem::create_directories(path.parent_path(), error);

        return WriteText(path, ToTsv(results));
    }

    std::optional<Baseline> ReadTsv(const std::filesystem::path& path)
    {
        std::ifstream file(path);
        if (!file)
            return std::nullopt;

        // The whole field must be a number ("nan" allowed for values); anything else rejects the file.
        const auto parse = [](const std::string& text, bool allowNan, double& value)
        {
            if (allowNan && text == "nan")
            {
                value = std::numeric_limits<double>::quiet_NaN();
                return true;
            }

            if (text.empty())
                return false;

            char* end = nullptr;
            value = std::strtod(text.c_str(), &end);
            return end == text.c_str() + text.size() && std::isfinite(value);
        };

        // A damaged baseline is refused as a whole: a skipped line would silently drop its metric from the gate.
        Baseline baseline;
        std::string line;
        while (std::getline(file, line))
        {
            if (line.empty() || line[0] == '#')
                continue;

            const std::vector<std::string> fields = Split(line, '\t');
            if (fields.size() != 5 || fields[0].empty() || baseline.contains(fields[0]))
                return std::nullopt;

            BaselineEntry entry;
            if (!parse(fields[1], true, entry.value))
                return std::nullopt;

            entry.unit = fields[2];

            if (fields[3] == "lower")
                entry.better = Better::Lower;
            else if (fields[3] == "higher")
                entry.better = Better::Higher;
            else
                return std::nullopt;

            if (fields[4] != "-")
            {
                double target = 0.0;
                if (!parse(fields[4], false, target))
                    return std::nullopt;

                entry.target = target;
            }

            baseline[fields[0]] = entry;
        }

        return baseline;
    }

    std::vector<Comparison> CompareToBaseline(const std::vector<ScenarioResult>& results, const Baseline& baseline)
    {
        std::vector<Comparison> comparisons;
        std::set<std::string> seen;

        for (const ScenarioResult& result : results)
        {
            for (const Measurement& measurement : result.measurements)
            {
                for (const Metric& metric : measurement.metrics)
                {
                    if (!metric.target)
                        continue;

                    const std::string key = MetricKey(result.id, measurement, metric);
                    seen.insert(key);

                    const auto it = baseline.find(key);
                    if (it == baseline.end())
                    {
                        comparisons.push_back({ key, Verdict::New, metric.value, std::numeric_limits<double>::quiet_NaN() });
                        continue;
                    }

                    comparisons.push_back({ key, Judge(metric, it->second), metric.value, it->second.value });
                }
            }
        }

        for (const auto& [key, entry] : baseline)
            if (entry.target && !seen.contains(key))
                comparisons.push_back({ key, Verdict::Missing, std::numeric_limits<double>::quiet_NaN(), entry.value });

        return comparisons;
    }

    bool WriteReport(const std::vector<ScenarioResult>& results, const ReportOptions& options, const Baseline* baseline)
    {
        std::error_code error;
        std::filesystem::create_directories(options.outDir, error);
        if (error)
            return false;

        std::map<std::string, Verdict> verdicts;
        if (baseline != nullptr)
            for (const Comparison& comparison : CompareToBaseline(results, *baseline))
                verdicts[comparison.key] = comparison.verdict;

        // report.json
        std::ostringstream json;
        json << "{\n  \"harness\": \"amplitude_fidelity\",\n  \"version\": 1,\n  \"git\": \"" << JsonEscape(options.gitSha)
             << "\",\n  \"build\": \"" << JsonEscape(options.buildMode) << "\",\n  \"resampler\": \"" << JsonEscape(options.resampler)
             << "\",\n  \"scenarios\": [";
        for (std::size_t s = 0; s < results.size(); ++s)
        {
            const ScenarioResult& result = results[s];
            json << (s > 0 ? "," : "") << "\n    {\"id\": \"" << JsonEscape(result.id) << "\", \"title\": \"" << JsonEscape(result.title)
                 << "\", \"measurements\": [";

            for (std::size_t i = 0; i < result.measurements.size(); ++i)
            {
                const Measurement& m = result.measurements[i];
                json << (i > 0 ? "," : "") << "\n      {\"variant\": \"" << JsonEscape(m.variant)
                     << "\", \"grid\": {\"blockSize\": " << m.point.blockSize << ", \"fps\": " << JsonNumber(m.point.fps)
                     << ", \"jitter\": " << (m.point.jitter ? "true" : "false") << ", \"outputRate\": " << m.point.outputRate
                     << "}, \"error\": " << (m.error.empty() ? std::string("null") : "\"" + JsonEscape(m.error) + "\"")
                     << ", \"metrics\": [";

                for (std::size_t k = 0; k < m.metrics.size(); ++k)
                {
                    const Metric& metric = m.metrics[k];
                    json << (k > 0 ? ", " : "") << "{\"name\": \"" << JsonEscape(metric.name)
                         << "\", \"value\": " << JsonNumber(metric.value) << ", \"unit\": \"" << JsonEscape(metric.unit)
                         << "\", \"better\": \"" << BetterName(metric.better)
                         << "\", \"target\": " << (metric.target ? JsonNumber(*metric.target) : std::string("null"))
                         << ", \"meets\": " << (metric.target ? (MeetsTarget(metric) ? "true" : "false") : "null") << "}";
                }

                json << "], \"events\": [";
                for (std::size_t k = 0; k < m.events.size(); ++k)
                {
                    const EventRecord& e = m.events[k];
                    json << (k > 0 ? ", " : "") << "{\"analyzer\": \"" << JsonEscape(e.analyzer) << "\", \"channel\": " << e.channel
                         << ", \"sample\": " << e.sample << ", \"levelDbfs\": " << JsonNumber(e.levelDbfs)
                         << ", \"aboveFloorDb\": " << JsonNumber(e.aboveFloorDb) << ", \"nearestAction\": \"" << JsonEscape(e.nearestAction)
                         << "\", \"actionOffset\": " << e.actionOffset << ", \"blockOffset\": " << e.blockOffset << "}";
                }

                json << "]}";
            }

            json << "\n    ]}";
        }

        json << "\n  ]\n}\n";

        bool ok = WriteText(options.outDir / "report.json", json.str());
        ok = WriteTsv(options.outDir / "metrics.tsv", results) && ok;

        // summary.md
        std::size_t gated = 0;
        std::size_t meets = 0;
        std::size_t errors = 0;
        std::size_t regressed = 0;
        for (const ScenarioResult& result : results)
        {
            for (const Measurement& m : result.measurements)
            {
                errors += m.error.empty() ? 0 : 1;
                for (const Metric& metric : m.metrics)
                {
                    if (!metric.target)
                        continue;

                    ++gated;
                    meets += MeetsTarget(metric) ? 1 : 0;
                }
            }
        }

        for (const auto& [key, verdict] : verdicts)
            regressed += verdict == Verdict::Regressed || verdict == Verdict::Missing ? 1 : 0;

        std::ostringstream md;
        md << "# Fidelity report\n\n";
        md << "Git `" << options.gitSha << "` · build " << options.buildMode << " · resampler " << options.resampler << "\n\n";
        md << "| Scenarios | Gated metrics | Meet target | Measurement errors |" << (baseline != nullptr ? " Regressed vs baseline |" : "")
           << "\n";
        md << "|---|---|---|---|" << (baseline != nullptr ? "---|" : "") << "\n";
        md << "| " << results.size() << " | " << gated << " | " << meets << " | " << errors << " |";
        if (baseline != nullptr)
            md << " " << regressed << " |";
        md << "\n";

        for (const ScenarioResult& result : results)
        {
            md << "\n## " << result.id << " — " << result.title << "\n\n";
            md << "| Variant | Grid | Metric | Value | Target | Status |\n|---|---|---|---|---|---|\n";

            std::vector<std::pair<const Measurement*, const EventRecord*>> events;
            for (const Measurement& m : result.measurements)
            {
                for (const Metric& metric : m.metrics)
                {
                    std::string status = "info";
                    if (metric.target)
                    {
                        status = MeetsTarget(metric) ? "meets" : "misses";
                        const auto it = verdicts.find(MetricKey(result.id, m, metric));
                        if (it != verdicts.end())
                            status += std::string(" (") + VerdictName(it->second) + ")";
                    }

                    md << "| " << m.variant << " | " << m.point.Key() << " | " << metric.name << " | " << Number(metric.value) << " "
                       << metric.unit << " | " << TargetText(metric) << " | " << status << " |\n";
                }

                for (const EventRecord& e : m.events)
                    events.emplace_back(&m, &e);
            }

            bool anyError = false;
            for (const Measurement& m : result.measurements)
            {
                if (m.error.empty())
                    continue;

                if (!anyError)
                    md << "\n**Errors**\n\n";

                anyError = true;
                md << "- `" << m.variant << "` `" << m.point.Key() << "`: " << m.error << "\n";
            }

            if (!events.empty())
            {
                std::sort(
                    events.begin(), events.end(),
                    [](const auto& a, const auto& b)
                    {
                        return a.second->levelDbfs > b.second->levelDbfs;
                    });

                md << "\n**Worst events**\n\n";
                for (std::size_t i = 0; i < std::min<std::size_t>(5, events.size()); ++i)
                {
                    const Measurement& m = *events[i].first;
                    const EventRecord& e = *events[i].second;
                    const double ms = m.capture.sampleRate > 0 ? 1000.0 * static_cast<double>(e.sample) / m.capture.sampleRate : 0.0;
                    md << "- `" << m.variant << "` `" << m.point.Key() << "` " << e.analyzer << " ch" << e.channel << " at sample "
                       << e.sample << " (" << Number(ms) << " ms): " << Number(e.levelDbfs) << " dBFS, " << Number(e.aboveFloorDb)
                       << " dB above floor";
                    if (!e.nearestAction.empty())
                        md << ", " << e.actionOffset << " samples after `" << e.nearestAction << "`";
                    md << ", block offset " << e.blockOffset << "\n";
                }
            }
        }

        ok = WriteText(options.outDir / "summary.md", md.str()) && ok;

        if (options.writeWavs)
        {
            for (const ScenarioResult& result : results)
            {
                for (const Measurement& m : result.measurements)
                {
                    if (m.capture.channels.empty())
                        continue;

                    const std::filesystem::path directory = options.outDir / "captures" / result.id;
                    std::filesystem::create_directories(directory, error);
                    const std::string stem = m.variant + "__" + m.point.Key();

                    ok = WriteWavFloat32(
                             directory / (stem + ".wav"), m.capture.sampleRate, static_cast<std::uint16_t>(m.capture.channels.size()),
                             Interleave(m.capture.channels)) &&
                        ok;

                    if (!m.residual.empty())
                        ok = WriteWavFloat32(
                                 directory / (stem + ".residual.wav"), m.capture.sampleRate, static_cast<std::uint16_t>(m.residual.size()),
                                 Interleave(m.residual)) &&
                            ok;
                }
            }
        }

        return ok;
    }
} // namespace SparkyStudios::Audio::Amplitude::Fidelity
