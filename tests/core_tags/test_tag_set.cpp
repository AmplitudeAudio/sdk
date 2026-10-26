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

#include <algorithm>
#include <vector>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Sound/TagSet.h>

#include "MemoryTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(MemoryTestCase, core_tags, tag_set)
    {
    public:
        void Run() override
        {
            MemoryManager::Initialize(nullptr);
            Check();
            MemoryManager::Deinitialize();
        }

        void Check()
        {
            const std::vector<AmTagID> a = { 1, 3 };
            const std::vector<AmTagID> b = { 2, 3 };
            const std::vector<AmTagID> none;

            const TagSet merged = UnionTags({ a, b });
            AM_EXPECT(merged != nullptr);
            AM_EXPECT(merged != nullptr && *merged == std::vector<AmTagID>({ 1, 2, 3 }));

            AM_EXPECT(UnionTags({ none, none }) == nullptr);
            AM_EXPECT(MakeTagSet(none) == nullptr);

            const TagSet made = MakeTagSet(a);
            AM_EXPECT(made != nullptr && *made == a);

            AM_EXPECT_NOT(TagSetContains(nullptr, 1));
            AM_EXPECT(TagSetContains(merged, 2));
            AM_EXPECT_NOT(TagSetContains(merged, 4));
        }
    };

    AM_REGISTER_TEST(core_tags, tag_set);

    AM_TEST_CASE(MemoryTestCase, core_tags, effective_tags_are_sorted_unique_for_many_tags)
    {
    public:
        void Run() override
        {
            MemoryManager::Initialize(nullptr);
            Check();
            MemoryManager::Deinitialize();
        }

        void Check()
        {
            std::vector<AmTagID> first;
            std::vector<AmTagID> second;
            for (AmTagID i = 1; i <= 300; ++i)
                first.push_back(i * 7);
            for (AmTagID i = 200; i <= 500; ++i)
                second.push_back(i * 7);

            const TagSet merged = UnionTags({ first, second });
            AM_EXPECT(merged != nullptr);
            if (merged == nullptr)
                return;

            AM_EXPECT_EQ(merged->size(), static_cast<AmSize>(500));
            AM_EXPECT(std::is_sorted(merged->begin(), merged->end()));
            AM_EXPECT(std::adjacent_find(merged->begin(), merged->end()) == merged->end());
            AM_EXPECT(TagSetContains(merged, 7));
            AM_EXPECT(TagSetContains(merged, 500 * 7));
            AM_EXPECT_NOT(TagSetContains(merged, 8));
        }
    };

    AM_REGISTER_TEST(core_tags, effective_tags_are_sorted_unique_for_many_tags);
} // namespace SparkyStudios::Audio::Amplitude::Tests
