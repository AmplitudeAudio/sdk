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

#include <cstddef>
#include <vector>

#include <Fidelity/ResamplerBench.h>

#include "PureUnitTestCase.h"
#include "TestRegistry.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    using namespace SparkyStudios::Audio::Amplitude::Fidelity;

    AM_TEST_CASE(PureUnitTestCase, fidelity_harness, resampler_bench_reports_every_preset)
    {
    public:
        void Run() override
        {
            const std::vector<Fidelity::ResamplerBenchRow> rows = Fidelity::RunResamplerBench(0.05);
            AM_EXPECT_EQ(std::size_t{ 16 }, rows.size()); // 4 presets x 4 ratios
            for (const auto& row : rows)
                AM_EXPECT(row.realtimeVoices > 0.0);
        }
    };

    AM_REGISTER_TEST(fidelity_harness, resampler_bench_reports_every_preset);
} // namespace SparkyStudios::Audio::Amplitude::Tests
