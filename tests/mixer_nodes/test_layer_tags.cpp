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

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include "EngineTestCase.h"
#include "NodeTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    namespace
    {
        constexpr const char* kTagA = "test.a";
        constexpr const char* kTagShared = "test.shared";
        constexpr const char* kTagCollection = "test.collection";
        constexpr const char* kTagSwitch = "test.switch";
        constexpr const char* kTagGrass = "test.grass";

        bool Contains(const std::vector<AmTagID>& tags, AmTagID tag)
        {
            return std::ranges::find(tags, tag) != tags.end();
        }
    } // namespace

    /// An engine whose pipeline carries a probe that records the tags of the layer it processes.
    class LayerTagsTestCase : public EngineTestCase
    {
    public:
        [[nodiscard]] const AmOsChar* GetConfigFile() const override
        {
            return AM_OS_STRING("tests.layer_tags.config.amconfig");
        }

    protected:
        /// Plays @p handle until the probe saw its layer, and returns what it recorded.
        template<typename Handle>
        bool PlayAndProbe(Handle handle, std::vector<AmTagID>& tags, std::vector<bool>& answers, const Entity& entity = Entity(nullptr))
        {
            TagProbeRecord::Get().Clear({ CreateTagId(kTagA), CreateTagId(kTagSwitch) });

            Channel channel = entity.Valid() ? amEngine->Play(handle, entity) : amEngine->Play(handle);
            const bool seen = WaitUntil(
                []()
                {
                    std::lock_guard lock(TagProbeRecord::Get().mutex);
                    return TagProbeRecord::Get().seen;
                });

            channel.Stop(0.0);

            std::lock_guard lock(TagProbeRecord::Get().mutex);
            tags = TagProbeRecord::Get().tags;
            answers = TagProbeRecord::Get().answers;
            return seen;
        }
    };

    AM_TEST_CASE(LayerTagsTestCase, mixer_nodes, layer_tags_standalone)
    {
    public:
        void Run() override
        {
            std::vector<AmTagID> tags;
            std::vector<bool> answers;
            AM_EXPECT(PlayAndProbe(amEngine->GetSoundHandle("test_sound_01"), tags, answers));

            AM_EXPECT_EQ(2, tags.size());
            AM_EXPECT(Contains(tags, CreateTagId(kTagA)));
            AM_EXPECT(Contains(tags, CreateTagId(kTagShared)));
            AM_EXPECT(!Contains(tags, CreateTagId(kTagSwitch)));

            AM_EXPECT_EQ(2, answers.size());
            AM_EXPECT(answers.size() == 2 && answers[0]); // HasTag(test.a)
            AM_EXPECT(answers.size() == 2 && !answers[1]); // HasTag(test.switch)
        }
    };

    AM_REGISTER_TEST(mixer_nodes, layer_tags_standalone);

    AM_TEST_CASE(LayerTagsTestCase, mixer_nodes, layer_tags_collection)
    {
    public:
        void Run() override
        {
            std::vector<AmTagID> tags;
            std::vector<bool> answers;
            AM_EXPECT(PlayAndProbe(amEngine->GetCollectionHandle("test_collection"), tags, answers));

            // The collection's own tags, plus those of the sound it picked (the sequence starts with sound 9991).
            AM_EXPECT_EQ(3, tags.size());
            AM_EXPECT(Contains(tags, CreateTagId(kTagA)));
            AM_EXPECT(Contains(tags, CreateTagId(kTagCollection)));
            AM_EXPECT(Contains(tags, CreateTagId(kTagShared)));
            AM_EXPECT(!Contains(tags, CreateTagId(kTagSwitch)));
        }
    };

    AM_REGISTER_TEST(mixer_nodes, layer_tags_collection);

    AM_TEST_CASE(LayerTagsTestCase, mixer_nodes, layer_tags_switch_container_over_collection)
    {
    public:
        void Run() override
        {
            std::vector<AmTagID> tags;
            std::vector<bool> answers;
            Entity entity = amEngine->AddEntity(100);
            AM_EXPECT(PlayAndProbe(amEngine->GetSwitchContainerHandle("footsteps"), tags, answers, entity));
            amEngine->RemoveEntity(100);

            // The switch container and the collection its default state maps to; the picked sound has none.
            AM_EXPECT_EQ(2, tags.size());
            AM_EXPECT(Contains(tags, CreateTagId(kTagSwitch)));
            AM_EXPECT(Contains(tags, CreateTagId(kTagGrass)));

            AM_EXPECT_EQ(2, answers.size());
            AM_EXPECT(answers.size() == 2 && !answers[0]); // HasTag(test.a)
            AM_EXPECT(answers.size() == 2 && answers[1]); // HasTag(test.switch)
        }
    };

    AM_REGISTER_TEST(mixer_nodes, layer_tags_switch_container_over_collection);

    // A layer with no sound reports nothing, and a layer written before tags existed still compiles.
    AM_TEST_CASE(NodeTestCase, mixer_nodes, layer_without_sound_has_no_tags)
    {
    public:
        void Run() override
        {
            const MinimalMockLayer layer;

            AM_EXPECT(layer.GetTags().empty());
            AM_EXPECT(!layer.HasTag(1));
        }
    };

    AM_REGISTER_TEST(mixer_nodes, layer_without_sound_has_no_tags);
} // namespace SparkyStudios::Audio::Amplitude::Tests
