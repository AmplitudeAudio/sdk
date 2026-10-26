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

#pragma once

#ifndef _AM_IMPLEMENTATION_SOUND_TAG_SET_H
#define _AM_IMPLEMENTATION_SOUND_TAG_SET_H

#include <algorithm>
#include <initializer_list>
#include <memory>
#include <span>
#include <vector>

#include <SparkyStudios/Audio/Amplitude/Core/Common/Types.h>
#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief An immutable, sorted and duplicate-free set of tag identifiers. A null set means no tags.
     */
    using TagSet = std::shared_ptr<const std::vector<AmTagID>>;

    /**
     * @brief Creates a tag set from an already sorted list of tags.
     *
     * @param[in] sorted The tags, sorted ascending and without duplicates.
     *
     * @return The tag set, or null if the input is empty.
     */
    [[nodiscard]] AM_INLINE TagSet MakeTagSet(std::span<const AmTagID> sorted)
    {
        if (sorted.empty())
            return nullptr;

        return ampoolshared(eMemoryPoolKind_Amplimix, std::vector<AmTagID>, sorted.begin(), sorted.end());
    }

    /**
     * @brief Creates the sorted union of several sorted tag lists.
     *
     * @param[in] sources The sorted tag lists to merge.
     *
     * @return The tag set holding every tag once, or null if the union is empty.
     */
    [[nodiscard]] AM_INLINE TagSet UnionTags(std::initializer_list<std::span<const AmTagID>> sources)
    {
        std::vector<AmTagID> merged;
        for (const auto& source : sources)
            merged.insert(merged.end(), source.begin(), source.end());

        std::sort(merged.begin(), merged.end());
        merged.erase(std::unique(merged.begin(), merged.end()), merged.end());

        return MakeTagSet(merged);
    }

    /**
     * @brief Checks whether a tag set contains a tag.
     *
     * @param[in] set The tag set, which may be null.
     * @param[in] tag The tag to look for.
     *
     * @return true if the tag is in the set, false otherwise or if the set is null.
     */
    [[nodiscard]] AM_INLINE bool TagSetContains(const TagSet& set, AmTagID tag)
    {
        return set != nullptr && std::binary_search(set->begin(), set->end(), tag);
    }
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_SOUND_TAG_SET_H
