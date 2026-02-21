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

#include "EngineTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(EngineTestCase, core_engine, accesses_same_collection_by_name_or_id)
    {
    public:
        void Run() override
        {
            CollectionHandle byName = amEngine->GetCollectionHandle("test_collection");
            AM_EXPECT_NOT(byName == nullptr);

            AmCollectionID id = byName->GetId();
            CollectionHandle byId = amEngine->GetCollectionHandle(id);
            AM_EXPECT_NOT(byId == nullptr);

            // Same handle returned for name and ID
            AM_EXPECT_EQ(byName, byId);
        }
    };

    AM_REGISTER_TEST(core_engine, accesses_same_collection_by_name_or_id);
} // namespace SparkyStudios::Audio::Amplitude::Tests
