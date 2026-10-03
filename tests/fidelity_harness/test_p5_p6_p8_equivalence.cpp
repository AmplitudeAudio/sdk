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

    AM_TEST_CASE(PureUnitTestCase, fidelity_harness, p5_p6_p8_equivalence)
    {
    public:
        void Run() override
        {
            ScenarioRegistry registry;
            RegisterAllScenarios(registry);

            for (const char* id : { "P5", "P6", "P8" })
            {
                const std::vector<const Scenario*> matches = registry.Match(id);
                AM_EXPECT_EQ(matches.size(), 1);
                if (matches.size() != 1)
                    continue;

                Measurement m;
                m.variant = "sine_997_44100";
                matches[0]->Measure(RunContext{}, m.point, m.variant, m);
                AM_EXPECT(m.error.empty());

                const Metric* residual = m.Find("null.residualPeakDbfs");
                AM_EXPECT(residual != nullptr && std::isfinite(residual->value));
                AM_EXPECT(m.residual.size() == 1 && !m.residual[0].empty());
            }
        }
    };

    AM_REGISTER_TEST_DESKTOP_ONLY(fidelity_harness, p5_p6_p8_equivalence);
} // namespace SparkyStudios::Audio::Amplitude::Tests
