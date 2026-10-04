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
#include "TestRegistry.h"

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

    AM_TEST_CASE(ComponentTestCase, mixer_pipeline, instance_stream_table_attaches_and_detaches)
    {
    public:
        void Run() override
        {
            InstanceStreamTable table(2);
            AM_EXPECT_EQ(table.GetSize(), 0);

            auto a = std::make_shared<VoiceStreamSlot>();
            auto b = std::make_shared<VoiceStreamSlot>();
            auto c = std::make_shared<VoiceStreamSlot>();

            // Invalid inputs are rejected.
            AM_EXPECT_NOT(table.Attach(kAmInvalidObjectId, a));
            AM_EXPECT_NOT(table.Attach(1, nullptr));

            // Attach and find.
            AM_EXPECT(table.Attach(1, a));
            AM_EXPECT(table.Attach(2, b));
            AM_EXPECT(table.Find(1) == a.get());
            AM_EXPECT(table.Find(2) == b.get());
            AM_EXPECT(table.Find(3) == nullptr);
            AM_EXPECT_EQ(table.GetSize(), 2);

            // Duplicate ID and a full table are rejected.
            AM_EXPECT_NOT(table.Attach(1, c));
            AM_EXPECT_NOT(table.Attach(3, c));
            AM_EXPECT(table.Find(3) == nullptr);

            // Detach returns the stream and frees the slot.
            AM_EXPECT(table.Detach(1) == a);
            AM_EXPECT(table.Detach(1) == nullptr);
            AM_EXPECT(table.Detach(kAmInvalidObjectId) == nullptr);
            AM_EXPECT_EQ(table.GetSize(), 1);
            AM_EXPECT(table.Find(1) == nullptr);
            AM_EXPECT(table.Find(2) == b.get());
            AM_EXPECT_EQ(a.use_count(), 1);

            // The freed slot is reused.
            AM_EXPECT(table.Attach(3, c));
            AM_EXPECT(table.Find(3) == c.get());
            AM_EXPECT_EQ(table.GetSize(), 2);
        }
    };

    AM_REGISTER_TEST(mixer_pipeline, instance_stream_table_attaches_and_detaches);
} // namespace SparkyStudios::Audio::Amplitude::Tests
