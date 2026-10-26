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

#include <vector>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Core/TagRegistry.h>

#include "PureUnitTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    namespace
    {
        const flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>>* Build(
            flatbuffers::FlatBufferBuilder& builder, const std::vector<const char*>& values)
        {
            std::vector<flatbuffers::Offset<flatbuffers::String>> offsets;
            for (const char* value : values)
                offsets.push_back(builder.CreateString(value));

            const auto vector = builder.CreateVector(offsets);
            builder.Finish(vector);
            return flatbuffers::GetRoot<flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>>>(builder.GetBufferPointer());
        }
    } // namespace

    AM_TEST_CASE(PureUnitTestCase, core_tags, tag_loading)
    {
    public:
        void Run() override
        {
            TagRegistry registry;

            {
                flatbuffers::FlatBufferBuilder builder;
                const auto* tags = Build(builder, { "b", "a", "b" });

                std::vector<AmTagID> out;
                AM_EXPECT(LoadTags(tags, "sound", "test", registry, out));
                AM_EXPECT_EQ(out.size(), static_cast<AmSize>(2));
                AM_EXPECT(out.size() == 2 && out[0] < out[1]);
            }

            {
                flatbuffers::FlatBufferBuilder builder;
                const auto* tags = Build(builder, { "" });

                std::vector<AmTagID> out;
                AM_EXPECT_NOT(LoadTags(tags, "sound", "test", registry, out));
                AM_EXPECT(out.empty());
            }

            {
                std::vector<AmTagID> out;
                AM_EXPECT(LoadTags(nullptr, "sound", "test", registry, out));
                AM_EXPECT(out.empty());
            }
        }
    };

    AM_REGISTER_TEST(core_tags, tag_loading);
} // namespace SparkyStudios::Audio::Amplitude::Tests
