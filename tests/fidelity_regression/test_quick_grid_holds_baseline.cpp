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
#include <Fidelity/Scenario.h>

#include "PureUnitTestCase.h"
#include "TestRegistry.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    using namespace SparkyStudios::Audio::Amplitude::Fidelity;

    // Metrics that meet their target must keep meeting it; the others must not get worse than the committed baseline.
    AM_TEST_CASE(PureUnitTestCase, fidelity_regression, quick_grid_holds_baseline)
    {
    public:
        void Run() override
        {
            const std::optional<Baseline> baseline = ReadTsv("../tests/fidelity/baseline.tsv");
            AM_EXPECT(baseline.has_value());
            if (!baseline)
                return;

            ScenarioRegistry registry;
            RegisterAllScenarios(registry);

            std::vector<ScenarioResult> results;
            for (const auto& scenario : registry.All())
                results.push_back(RunScenario(*scenario, RunContext{}));

            // Kept for inspection when the gate fails.
            WriteTsv("fidelity/regression-current.tsv", results);

            for (const ScenarioResult& result : results)
            {
                for (const Measurement& m : result.measurements)
                {
                    if (!m.error.empty())
                        amLogError("fidelity: %s %s %s: %s", result.id.c_str(), m.variant.c_str(), m.point.Key().c_str(), m.error.c_str());

                    AM_EXPECT(m.error.empty());
                }
            }

            for (const Comparison& comparison : CompareToBaseline(results, *baseline))
            {
                const bool held = comparison.verdict != Verdict::Regressed && comparison.verdict != Verdict::Missing;
                if (!held)
                    amLogError(
                        "fidelity: %s %s (current %f, baseline %f)", VerdictName(comparison.verdict), comparison.key.c_str(),
                        comparison.current, comparison.baseline);

                AM_EXPECT(held);
            }
        }
    };

    AM_REGISTER_TEST_DESKTOP_ONLY(fidelity_regression, quick_grid_holds_baseline);
} // namespace SparkyStudios::Audio::Amplitude::Tests
