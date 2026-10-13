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

#include <filesystem>
#include <limits>

#include <Fidelity/Report.h>

#include "PureUnitTestCase.h"
#include "TestRegistry.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    using namespace SparkyStudios::Audio::Amplitude::Fidelity;

    namespace
    {
        std::vector<ScenarioResult> OneMetric(const std::string& name, double value, const std::string& unit, std::optional<double> target)
        {
            Measurement m;
            m.variant = "v";
            m.Add(name, value, unit, Better::Lower, target);
            return { ScenarioResult{ "P1", "Test", { m } } };
        }

        Verdict VerdictFor(double baselineValue, double current, const std::string& unit, double target)
        {
            const auto baselineResults = OneMetric("m", baselineValue, unit, target);
            Baseline baseline;
            const Measurement& b = baselineResults[0].measurements[0];
            baseline[MetricKey("P1", b, b.metrics[0])] = BaselineEntry{ baselineValue, unit, Better::Lower, target };

            const std::vector<Comparison> comparisons = CompareToBaseline(OneMetric("m", current, unit, target), baseline);
            return comparisons.size() == 1 ? comparisons[0].verdict : Verdict::Missing;
        }
    } // namespace

    AM_TEST_CASE(PureUnitTestCase, fidelity_harness, report_baseline)
    {
    public:
        void Run() override
        {
            // TSV round trip, NaN included.
            Measurement m;
            m.variant = "sine_997_44100";
            m.Add("spectrum.thdnDb", -98.25, "dB", Better::Lower, -100.0);
            m.Add("response.minus3dBHz", 20500.0, "Hz", Better::Higher);
            m.Add("broken", std::numeric_limits<double>::quiet_NaN(), "dB", Better::Lower, -90.0);
            const std::vector<ScenarioResult> results = { ScenarioResult{ "P1", "Resampling", { m } } };

            const std::filesystem::path directory = std::filesystem::temp_directory_path() / "amplitude_fidelity_tests" / "tsv";
            std::filesystem::remove_all(directory);
            std::filesystem::create_directories(directory);
            AM_EXPECT(WriteTsv(directory / "metrics.tsv", results));

            const std::optional<Baseline> read = ReadTsv(directory / "metrics.tsv");
            AM_EXPECT(read.has_value());
            if (read.has_value())
            {
                AM_EXPECT_EQ(read->size(), 3);
                const BaselineEntry& thdn = read->at("P1|sine_997_44100|b1024_f60_j0_r48000|spectrum.thdnDb");
                AM_EXPECT(std::abs(thdn.value - (-98.25)) < 1e-6);
                AM_EXPECT(thdn.unit == "dB" && thdn.better == Better::Lower && thdn.target == -100.0);
                const BaselineEntry& cutoff = read->at("P1|sine_997_44100|b1024_f60_j0_r48000|response.minus3dBHz");
                AM_EXPECT(cutoff.better == Better::Higher && !cutoff.target.has_value());
                AM_EXPECT(std::isnan(read->at("P1|sine_997_44100|b1024_f60_j0_r48000|broken").value));
            }

            AM_EXPECT(!ReadTsv(directory / "missing.tsv").has_value());
            std::filesystem::remove_all(directory);

            // Verdicts.
            AM_EXPECT(VerdictFor(-105.0, -104.0, "dB", -100.0) == Verdict::MeetsTarget);
            AM_EXPECT(VerdictFor(-105.0, -99.0, "dB", -100.0) == Verdict::Regressed);
            AM_EXPECT(VerdictFor(-90.0, -89.7, "dB", -100.0) == Verdict::NoWorse);
            AM_EXPECT(VerdictFor(-90.0, -89.0, "dB", -100.0) == Verdict::Regressed);
            AM_EXPECT(VerdictFor(-90.0, -95.0, "dB", -100.0) == Verdict::Improved);
            AM_EXPECT(VerdictFor(3.0, 3.0, "count", 0.0) == Verdict::NoWorse);
            AM_EXPECT(VerdictFor(3.0, 4.0, "count", 0.0) == Verdict::Regressed);
            AM_EXPECT(VerdictFor(-90.0, std::numeric_limits<double>::quiet_NaN(), "dB", -100.0) == Verdict::Regressed);

            // New, Missing, and ungated metrics.
            Baseline baseline;
            baseline["P1|v|b1024_f60_j0_r48000|gone"] = BaselineEntry{ 1.0, "count", Better::Lower, 0.0 };
            baseline["P1|v|b1024_f60_j0_r48000|info"] = BaselineEntry{ 1.0, "Hz", Better::Lower, std::nullopt };
            Measurement current;
            current.variant = "v";
            current.Add("fresh", 0.0, "count", Better::Lower, 0.0);
            current.Add("info", 5.0, "Hz", Better::Lower);
            const std::vector<Comparison> comparisons = CompareToBaseline({ ScenarioResult{ "P1", "Test", { current } } }, baseline);
            AM_EXPECT_EQ(comparisons.size(), 2);
            bool sawNew = false;
            bool sawMissing = false;
            for (const Comparison& c : comparisons)
            {
                sawNew = sawNew || (c.verdict == Verdict::New && c.key.ends_with("|fresh"));
                sawMissing = sawMissing || (c.verdict == Verdict::Missing && c.key.ends_with("|gone"));
            }
            AM_EXPECT(sawNew && sawMissing);

            AM_EXPECT(JsonEscape("a\"b\\c\n") == "a\\\"b\\\\c\\n");
            AM_EXPECT(ToleranceFor("dB") == 0.5 && ToleranceFor("count") == 0.0 && ToleranceFor("cents") == 0.1);
        }
    };

    AM_REGISTER_TEST_DESKTOP_ONLY(fidelity_harness, report_baseline);
} // namespace SparkyStudios::Audio::Amplitude::Tests
