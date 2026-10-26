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

#ifndef _AM_CORE_TAG_REGISTRY_H
#define _AM_CORE_TAG_REGISTRY_H

#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <flatbuffers/flatbuffers.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief Maps tag identifiers to their strings, and detects hash collisions.
     */
    class TagRegistry
    {
    public:
        using HashFn = AmTagID (*)(std::string_view);

        explicit TagRegistry(HashFn hash = &CreateTagId);

        /**
         * @brief Registers a tag.
         *
         * @param tag The tag string.
         * @param id Receives the tag identifier.
         *
         * @return `false` (and logs an error) if the tag is empty, hashes to 0, or collides with another tag.
         */
        bool Register(std::string_view tag, AmTagID& id);

        /**
         * @brief Removes all registered tags.
         */
        void Clear();

        /**
         * @brief Gets the number of registered tags.
         */
        [[nodiscard]] AmSize Size() const;

    private:
        HashFn _hash;
        mutable std::mutex _mutex;
        std::unordered_map<AmTagID, std::string> _tags;
    };

    /**
     * @brief Registers the tags of an asset and collects their ids.
     *
     * @param tags The tag strings from the asset definition. May be null.
     * @param assetKind The kind of asset, for error messages.
     * @param assetName The name of the asset, for error messages.
     * @param registry The registry to register the tags in.
     * @param out Receives the ids, sorted ascending without duplicates. Left untouched on failure.
     *
     * @return `false` if a tag could not be registered.
     */
    bool LoadTags(
        const flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>>* tags,
        const char* assetKind,
        const char* assetName,
        TagRegistry& registry,
        std::vector<AmTagID>& out);
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_CORE_TAG_REGISTRY_H
