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

#include "PureUnitTestCase.h"
#include "TestRegistry.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    using namespace SparkyStudios::Audio::Amplitude::Fidelity;

    AM_TEST_CASE(PureUnitTestCase, fidelity_harness, p1_resampling_quality)
    {
    public:
        void Run() override
        {
            ScenarioRegistry registry;
            RegisterAllScenarios(registry);
            const std::vector<const Scenario*> matches = registry.Match("P1");
            AM_EXPECT_EQ(matches.size(), 1);
            if (matches.size() != 1)
                return;

            const Scenario& scenario = *matches[0];
            const auto measure = [&](const std::string& variant)
            {
                Measurement m;
                m.variant = variant;
                scenario.Measure(RunContext{}, m.point, variant, m);
                return m;
            };

            const auto expectFinite = [this](const Measurement& m, const char* name)
            {
                const Metric* metric = m.Find(name);
                AM_EXPECT(metric != nullptr && std::isfinite(metric->value));
            };

            const Measurement tone = measure("sine_997_44100");
            AM_EXPECT(tone.error.empty());
            for (const char* name : { "determinism.mismatches", "integrity.nan", "spectrum.thdnDb", "spectrum.worstSpurDbc",
                                      "spectrum.frequencyErrorCents", "click.worstDbfs" })
                expectFinite(tone, name);
            AM_EXPECT(!tone.capture.channels.empty());

            const Measurement stereo = measure("sine_997_48000_stereo");
            AM_EXPECT(stereo.error.empty());
            expectFinite(stereo, "spectrum.thdnDb.L");
            expectFinite(stereo, "spectrum.thdnDb.R");

            const Measurement alias = measure("sine_30000_96000");
            AM_EXPECT(alias.error.empty());
            expectFinite(alias, "alias.levelDbc");

            const Measurement sweep = measure("sweep_44100");
            AM_EXPECT(sweep.error.empty());
            expectFinite(sweep, "response.rippleDb");
        }
    };

    AM_REGISTER_TEST_DESKTOP_ONLY(fidelity_harness, p1_resampling_quality);
} // namespace SparkyStudios::Audio::Amplitude::Tests
