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

    AM_TEST_CASE(PureUnitTestCase, fidelity_harness, p2_p3_transport)
    {
    public:
        void Run() override
        {
            ScenarioRegistry registry;
            RegisterAllScenarios(registry);

            const auto run = [&](const char* id, const std::string& variant)
            {
                Measurement m;
                m.variant = variant;
                const std::vector<const Scenario*> matches = registry.Match(id);
                AM_EXPECT_EQ(matches.size(), 1);
                if (matches.size() == 1)
                    matches[0]->Measure(RunContext{}, m.point, variant, m);

                return m;
            };

            const auto expectFinite = [this](const Measurement& m, const char* name)
            {
                const Metric* metric = m.Find(name);
                AM_EXPECT(metric != nullptr && std::isfinite(metric->value));
            };

            const Measurement fade = run("P2", "default_fade");
            AM_EXPECT(fade.error.empty());
            for (const char* name : { "click.worstDbfs", "stop.latencyMs", "stop.fadeMs", "stop.fadeCurveErrorDb", "stop.tailDbfs" })
                expectFinite(fade, name);

            const Measurement cut = run("P2", "zero_fade");
            AM_EXPECT(cut.error.empty());
            expectFinite(cut, "stop.tailDbfs");
            AM_EXPECT(cut.Find("stop.fadeCurveErrorDb") == nullptr);

            const Measurement pause = run("P3", "pause_resume");
            AM_EXPECT(pause.error.empty());
            expectFinite(pause, "resume.positionErrorSamples");

            const Measurement seek = run("P3", "seek_forward");
            AM_EXPECT(seek.error.empty());
            expectFinite(seek, "seek.latencyMs");
        }
    };

    AM_REGISTER_TEST_DESKTOP_ONLY(fidelity_harness, p2_p3_transport);
} // namespace SparkyStudios::Audio::Amplitude::Tests
