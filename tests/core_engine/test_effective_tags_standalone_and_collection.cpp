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

#include <Sound/Collection.h>
#include <Sound/Sound.h>
#include <Sound/TagSet.h>

#include "EngineTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    AM_TEST_CASE(EngineTestCase, core_engine, effective_tags_standalone_and_collection)
    {
    public:
        void Run() override
        {
            auto* tagged = static_cast<SoundImpl*>(amEngine->GetSoundHandle("test_sound_01"));
            auto* untagged = static_cast<SoundImpl*>(amEngine->GetSoundHandle("test_sound_02"));
            auto* collection = static_cast<CollectionImpl*>(amEngine->GetCollectionHandle("test_collection"));
            AM_EXPECT(tagged != nullptr && untagged != nullptr && collection != nullptr);
            if (tagged == nullptr || untagged == nullptr || collection == nullptr)
                return;

            const AmTagID a = CreateTagId("test.a");
            const AmTagID shared = CreateTagId("test.shared");
            const AmTagID fromCollection = CreateTagId("test.collection");

            SoundInstance* standalone = tagged->CreateInstance();
            const TagSet& standaloneTags = standalone->GetSettings().m_tags;
            AM_EXPECT(standaloneTags != nullptr);
            AM_EXPECT_EQ(standaloneTags ? standaloneTags->size() : 0, static_cast<AmSize>(2));
            AM_EXPECT(TagSetContains(standaloneTags, a));
            AM_EXPECT(TagSetContains(standaloneTags, shared));
            AM_EXPECT_NOT(TagSetContains(standaloneTags, fromCollection));
            SoundImpl::DestroyInstance(standalone);

            SoundInstance* contained = tagged->CreateInstance(collection);
            const TagSet& containedTags = contained->GetSettings().m_tags;
            AM_EXPECT(containedTags != nullptr);
            AM_EXPECT_EQ(containedTags ? containedTags->size() : 0, static_cast<AmSize>(3));
            AM_EXPECT(TagSetContains(containedTags, a));
            AM_EXPECT(TagSetContains(containedTags, shared));
            AM_EXPECT(TagSetContains(containedTags, fromCollection));
            SoundImpl::DestroyInstance(contained);

            SoundInstance* plain = untagged->CreateInstance();
            AM_EXPECT(plain->GetSettings().m_tags == nullptr);
            SoundImpl::DestroyInstance(plain);
        }
    };

    AM_REGISTER_TEST(core_engine, effective_tags_standalone_and_collection);
} // namespace SparkyStudios::Audio::Amplitude::Tests
