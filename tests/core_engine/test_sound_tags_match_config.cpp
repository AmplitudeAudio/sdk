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

#include "EngineTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    namespace
    {
        std::vector<AmTagID> SortedIds(std::initializer_list<const char*> tags)
        {
            std::vector<AmTagID> ids;
            for (const char* tag : tags)
                ids.push_back(CreateTagId(tag));
            std::ranges::sort(ids);
            return ids;
        }

        bool SameTags(std::span<const AmTagID> actual, const std::vector<AmTagID>& expected)
        {
            return std::ranges::equal(actual, expected);
        }
    } // namespace

    AM_TEST_CASE(EngineTestCase, core_engine, sound_tags_match_config)
    {
    public:
        void Run() override
        {
            SoundHandle sound = amEngine->GetSoundHandle("test_sound_01");
            AM_EXPECT_NOT(sound == nullptr);
            AM_EXPECT(SameTags(sound->GetTags(), SortedIds({ "test.a", "test.shared" })));
            AM_EXPECT(sound->HasTag(CreateTagId("test.a")));
            AM_EXPECT(sound->HasTag(CreateTagId("test.shared")));
            AM_EXPECT_NOT(sound->HasTag(CreateTagId("test.collection")));

            CollectionHandle collection = amEngine->GetCollectionHandle("test_collection");
            AM_EXPECT_NOT(collection == nullptr);
            AM_EXPECT(SameTags(collection->GetTags(), SortedIds({ "test.collection", "test.shared" })));
            AM_EXPECT(collection->HasTag(CreateTagId("test.collection")));

            SwitchContainerHandle container = amEngine->GetSwitchContainerHandle("footsteps");
            AM_EXPECT_NOT(container == nullptr);
            AM_EXPECT(SameTags(container->GetTags(), SortedIds({ "test.switch" })));
            AM_EXPECT(container->HasTag(CreateTagId("test.switch")));
        }
    };

    AM_REGISTER_TEST(core_engine, sound_tags_match_config);

    AM_TEST_CASE(EngineTestCase, core_engine, sound_without_tags_has_no_tags)
    {
    public:
        void Run() override
        {
            SoundHandle sound = amEngine->GetSoundHandle("test_sound_02");
            AM_EXPECT_NOT(sound == nullptr);
            AM_EXPECT(sound->GetTags().empty());
            AM_EXPECT_NOT(sound->HasTag(CreateTagId("test.a")));
        }
    };

    AM_REGISTER_TEST(core_engine, sound_without_tags_has_no_tags);
} // namespace SparkyStudios::Audio::Amplitude::Tests
