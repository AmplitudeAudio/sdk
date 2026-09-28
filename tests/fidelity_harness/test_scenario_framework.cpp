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

#include <set>
#include <stdexcept>

#include <Fidelity/Scenario.h>

#include "PureUnitTestCase.h"
#include "TestRegistry.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    using namespace SparkyStudios::Audio::Amplitude::Fidelity;

    namespace
    {
        class FakeScenario final : public Scenario
        {
        public:
            explicit FakeScenario(std::string id)
                : _id(std::move(id))
            {}

            [[nodiscard]] std::string Id() const override
            {
                return _id;
            }

            [[nodiscard]] std::string Title() const override
            {
                return "Fake";
            }

            [[nodiscard]] std::uint32_t Dimensions() const override
            {
                return kGridBlockSize;
            }

            [[nodiscard]] std::vector<std::string> Variants() const override
            {
                return { "a", "b", "boom" };
            }

            void Measure(const RunContext&, const GridPoint& point, const std::string& variant, Measurement& out) const override
            {
                if (variant == "boom")
                    throw std::runtime_error("boom");

                out.Add("value", static_cast<double>(point.blockSize), "count", Better::Lower, 2048.0);
            }

        private:
            std::string _id;
        };
    } // namespace

    AM_TEST_CASE(PureUnitTestCase, fidelity_harness, scenario_framework)
    {
    public:
        void Run() override
        {
            AM_EXPECT(GridPoint{}.Key() == "b1024_f60_j0_r48000");
            AM_EXPECT_EQ(ExpandGrid(GridMode::Quick, kGridBlockSize | kGridFps | kGridJitter | kGridOutputRate).size(), 1);
            AM_EXPECT_EQ(ExpandGrid(GridMode::Full, 0).size(), 1);

            const std::vector<GridPoint> full = ExpandGrid(GridMode::Full, kGridBlockSize | kGridOutputRate);
            AM_EXPECT_EQ(full.size(), 6);
            std::set<std::string> keys;
            for (const GridPoint& point : full)
                keys.insert(point.Key());
            AM_EXPECT_EQ(keys.size(), 6);
            AM_EXPECT_EQ(ExpandGrid(GridMode::Full, kGridFps | kGridJitter).size(), 6);

            AM_EXPECT(GlobMatch("P*", "P1"));
            AM_EXPECT(GlobMatch("*", ""));
            AM_EXPECT(GlobMatch("P?", "P1"));
            AM_EXPECT(!GlobMatch("P?", "P12"));
            AM_EXPECT(!GlobMatch("P1", "P2"));

            ScenarioRegistry registry;
            registry.Add(std::make_unique<FakeScenario>("P1"));
            registry.Add(std::make_unique<FakeScenario>("Q1"));
            AM_EXPECT_EQ(registry.All().size(), 2);
            AM_EXPECT_EQ(registry.Match("P*").size(), 1);
            AM_EXPECT(registry.Match("Z*").empty());

            // A throwing variant is recorded as an error; the others still run.
            RunContext context;
            context.grid = GridMode::Full;
            const ScenarioResult result = RunScenario(*registry.All()[0], context);
            AM_EXPECT(result.id == "P1");
            AM_EXPECT_EQ(result.measurements.size(), 9); // 3 block sizes x 3 variants
            for (const Measurement& m : result.measurements)
            {
                if (m.variant == "boom")
                {
                    AM_EXPECT(m.error.find("boom") != std::string::npos);
                    continue;
                }

                AM_EXPECT(m.error.empty());
                const Metric* metric = m.Find("value");
                AM_EXPECT(metric != nullptr && metric->value == static_cast<double>(m.point.blockSize));
                AM_EXPECT(metric != nullptr && MeetsTarget(*metric) == (m.point.blockSize <= 2048));
            }

            AM_EXPECT(MeetsTarget(-101.0, Better::Lower, -100.0));
            AM_EXPECT(!MeetsTarget(-99.0, Better::Lower, -100.0));
            AM_EXPECT(MeetsTarget(20000.0, Better::Higher, 19000.0));
            AM_EXPECT(!MeetsTarget(std::nan(""), Better::Lower, 0.0));
            AM_EXPECT(!MeetsTarget(Metric{ "info", 1.0, "dB", Better::Lower, std::nullopt }));

            // Events are tied to the latest action and block before them.
            Capture capture;
            capture.markers = {
                { 0, MarkerKind::Frame, {} },         { 0, MarkerKind::Block, {} },    { 800, MarkerKind::Frame, {} },
                { 800, MarkerKind::Action, "play" },  { 1024, MarkerKind::Block, {} }, { 1600, MarkerKind::Frame, {} },
                { 1600, MarkerKind::Action, "stop" }, { 2048, MarkerKind::Block, {} },
            };
            std::vector<EventRecord> events(1);
            events[0].sample = 1700;
            AnnotateEvents(events, capture);
            AM_EXPECT(events[0].nearestAction == "stop");
            AM_EXPECT_EQ(events[0].actionOffset, 100);
            AM_EXPECT_EQ(events[0].blockOffset, 676);
        }
    };

    AM_REGISTER_TEST(fidelity_harness, scenario_framework);
} // namespace SparkyStudios::Audio::Amplitude::Tests
