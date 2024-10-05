// Copyright (c) 2021-present Sparky Studios. All rights reserved.
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

#include <SparkyStudios/Audio/Amplitude/Core/Version.h>

namespace SparkyStudios::Audio::Amplitude
{
// Turn X into a string literal.
#define AM_STRING_EXPAND(X) #X
#define AM_STRING(X) AM_STRING_EXPAND(X)

    /**
     * @brief String which identifies the current version of Amplitude.
     */
    // clang-format off
    static const AmVersion kVersion = {
        AM_VERSION_MAJOR,
        AM_VERSION_MINOR,
        AM_VERSION_PATCH,
        "Amplitude "
        AM_STRING(AM_VERSION_MAJOR) "."
        AM_STRING(AM_VERSION_MINOR) "."
        AM_STRING(AM_VERSION_PATCH)
    };
    // clang-format on

    const AmVersion& GetVersion()
    {
        return kVersion;
    }
} // namespace SparkyStudios::Audio::Amplitude