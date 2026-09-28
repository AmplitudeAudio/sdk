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

#include <memory>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Mixer/InstancePipelineTable.h>

#include "ComponentTestCase.h"
#include "DSPTestCase.h"
#include "TestRegistry.h"
#include "TestUtils.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    namespace
    {
        class FakePipelineInstance final : public PipelineInstance
        {
        public:
            void Execute(AudioBuffer&, AudioBuffer&) override
            {}

            void Reset() override
            {}

            [[nodiscard]] std::shared_ptr<NodeInstance> GetNode(AmObjectID) const override
            {
                return nullptr;
            }

            void Configure(AmUInt64, AmUInt16, AmUInt64, AmUInt16) override
            {}
        };
    } // namespace

    AM_TEST_CASE(ComponentTestCase, mixer_pipeline, instance_pipeline_table)
    {
    public:
        void Run() override
        {
            InstancePipelineTable table(2);
            AM_EXPECT_EQ(table.GetCapacity(), 2);
            AM_EXPECT_EQ(table.GetSize(), 0);

            auto a = std::make_shared<FakePipelineInstance>();
            auto b = std::make_shared<FakePipelineInstance>();
            auto c = std::make_shared<FakePipelineInstance>();

            // Invalid inputs are rejected.
            AM_EXPECT_NOT(table.Attach(kAmInvalidObjectId, a));
            AM_EXPECT_NOT(table.Attach(1, nullptr));

            // Attach and find.
            AM_EXPECT(table.Attach(1, a));
            AM_EXPECT(table.Find(1) == a.get());
            AM_EXPECT(table.Find(2) == nullptr);

            // Duplicate ID rejected.
            AM_EXPECT_NOT(table.Attach(1, b));

            // Fill, then reject when full.
            AM_EXPECT(table.Attach(2, b));
            AM_EXPECT_EQ(table.GetSize(), 2);
            AM_EXPECT_NOT(table.Attach(3, c));
            AM_EXPECT(table.Find(3) == nullptr);

            // Detach returns the pipeline and frees the slot for reuse.
            AM_EXPECT(table.Detach(1) == a);
            AM_EXPECT(table.Detach(1) == nullptr);
            AM_EXPECT(table.Find(1) == nullptr);
            AM_EXPECT(table.Attach(3, c));
            AM_EXPECT(table.Find(3) == c.get());
            AM_EXPECT(table.Find(2) == b.get());

            // Clear releases everything; capacity is unchanged.
            table.Clear();
            AM_EXPECT_EQ(table.GetSize(), 0);
            AM_EXPECT(table.Find(2) == nullptr);
            AM_EXPECT(table.Find(3) == nullptr);
            AM_EXPECT_EQ(table.GetCapacity(), 2);
            AM_EXPECT_EQ(b.use_count(), 1);
            AM_EXPECT_EQ(c.use_count(), 1);
        }
    };

    AM_REGISTER_TEST(mixer_pipeline, instance_pipeline_table);

    AM_TEST_CASE(DSPTestCase, mixer_pipeline, instance_pipeline_table_owns_converters)
    {
    public:
        void Run() override
        {
            InstancePipelineTable table(3);

            auto a = std::make_shared<FakePipelineInstance>();
            auto b = std::make_shared<FakePipelineInstance>();
            auto converterA = std::make_shared<AudioConverter>();
            auto converterB = std::make_shared<AudioConverter>();

            AM_EXPECT(table.Attach(1, a, converterA));
            AM_EXPECT(table.Attach(2, b, converterB));
            AM_EXPECT(table.Attach(3, std::make_shared<FakePipelineInstance>()));

            // Each instance finds its own converter; one attached without a converter has none.
            AM_EXPECT(table.FindConverter(1) == converterA.get());
            AM_EXPECT(table.FindConverter(2) == converterB.get());
            AM_EXPECT(table.FindConverter(3) == nullptr);
            AM_EXPECT(table.FindConverter(4) == nullptr);

            AmSize visited = 0;
            table.ForEachConverter(
                [&](AudioConverter& converter)
                {
                    AM_EXPECT(&converter == converterA.get() || &converter == converterB.get());
                    ++visited;
                });
            AM_EXPECT_EQ(visited, 2);

            // Detaching an instance releases its converter with its pipeline.
            AM_EXPECT(table.Detach(1) == a);
            AM_EXPECT(table.FindConverter(1) == nullptr);
            AM_EXPECT_EQ(converterA.use_count(), 1);

            table.Clear();
            AM_EXPECT_EQ(converterB.use_count(), 1);
        }
    };

    AM_REGISTER_TEST(mixer_pipeline, instance_pipeline_table_owns_converters);
} // namespace SparkyStudios::Audio::Amplitude::Tests
