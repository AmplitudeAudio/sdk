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

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Sound/Schedulers/SequenceScheduler.h>

#include "collection_definition_generated.h"
#include "EngineTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(EngineTestCase, core_sound_scheduler, sequence_scheduler_handles_empty_list)
    {
    public:
        void Run() override
        {
            flatbuffers::FlatBufferBuilder builder;
            const auto name = builder.CreateString("empty_sequence_test");
            CollectionDefinitionBuilder collection(builder);
            collection.add_id(424243);
            collection.add_name(name);
            builder.Finish(collection.Finish());

            SequenceScheduler scheduler(nullptr);
            scheduler.Init(flatbuffers::GetRoot<CollectionDefinition>(builder.GetBufferPointer()));

            for (int i = 0; i < 3; ++i)
                AM_EXPECT_EQ(scheduler.Select({}), nullptr);
        }
    };

    AM_REGISTER_TEST(core_sound_scheduler, sequence_scheduler_handles_empty_list);
} // namespace SparkyStudios::Audio::Amplitude::Tests
