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

#ifndef _AM_CORE_TAG_H
#define _AM_CORE_TAG_H

#include <string_view>

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief Computes the identifier of a tag.
     *
     * The identifier is the 64-bit FNV-1a hash of the bytes of the tag string.
     *
     * @param tag The tag string.
     *
     * @return The identifier of the tag.
     *
     * @ingroup core
     */
    constexpr AmTagID CreateTagId(std::string_view tag)
    {
        AmTagID hash = 0xcbf29ce484222325ULL;

        for (const char c : tag)
        {
            hash ^= static_cast<AmUInt8>(c);
            hash *= 0x100000001b3ULL;
        }

        return hash;
    }
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_CORE_TAG_H
