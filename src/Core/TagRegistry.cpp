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

#include <Core/TagRegistry.h>

namespace SparkyStudios::Audio::Amplitude
{
    TagRegistry::TagRegistry(HashFn hash)
        : _hash(hash)
        , _mutex()
        , _tags()
    {}

    bool TagRegistry::Register(std::string_view tag, AmTagID& id)
    {
        if (tag.empty())
        {
            amLogError("A tag cannot be an empty string.");
            return false;
        }

        const AmTagID hash = _hash(tag);
        if (hash == 0)
        {
            amLogError("The tag '%.*s' hashes to 0, which is reserved.", static_cast<int>(tag.size()), tag.data());
            return false;
        }

        std::lock_guard lock(_mutex);

        const auto [it, inserted] = _tags.try_emplace(hash, tag);
        if (!inserted && it->second != tag)
        {
            amLogError(
                "The tags '%s' and '%.*s' have the same id.", it->second.c_str(), static_cast<int>(tag.size()), tag.data());
            return false;
        }

        id = hash;
        return true;
    }

    void TagRegistry::Clear()
    {
        std::lock_guard lock(_mutex);
        _tags.clear();
    }

    AmSize TagRegistry::Size() const
    {
        std::lock_guard lock(_mutex);
        return _tags.size();
    }

    bool LoadTags(
        const flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>>* tags,
        const char* assetKind,
        const char* assetName,
        TagRegistry& registry,
        std::vector<AmTagID>& out)
    {
        if (tags == nullptr)
            return true;

        std::vector<AmTagID> ids;
        ids.reserve(tags->size());

        for (flatbuffers::uoffset_t i = 0; i < tags->size(); ++i)
        {
            const auto* tag = tags->Get(i);
            AmTagID id = 0;
            if (tag == nullptr || !registry.Register(tag->string_view(), id))
            {
                amLogError("Failed to load the tags of the %s '%s'.", assetKind, assetName);
                return false;
            }

            ids.push_back(id);
        }

        std::sort(ids.begin(), ids.end());
        ids.erase(std::unique(ids.begin(), ids.end()), ids.end());

        out.insert(out.end(), ids.begin(), ids.end());
        return true;
    }
} // namespace SparkyStudios::Audio::Amplitude
